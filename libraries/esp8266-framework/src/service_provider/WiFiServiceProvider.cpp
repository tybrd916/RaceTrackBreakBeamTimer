/******************************* WiFi service *********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "WiFiServiceProvider.h"

extern "C" void preinit() {
#ifdef ENABLE_NAPT_FEATURE_LWIP_V2
  WiFiInterface::preinitWiFiOff();
#endif
  ESP.eraseConfig();
	uint8_t sta_mac[6];
  wifi_get_macaddr(STATION_IF, sta_mac);
  sta_mac[0] +=2;
	wifi_set_macaddr(SOFTAP_IF, sta_mac);
}

__status_wifi_t __status_wifi = {
  false, false, 0, {0}
};

/**
 * WiFiServiceProvider constructor.
 */
WiFiServiceProvider::WiFiServiceProvider():
  m_wifi_connection_timeout(WIFI_STATION_CONNECT_ATTEMPT_TIMEOUT),
  m_wifi(nullptr)
{
  memset(m_temp_mac, 0, 6);
}

/**
 * WiFiServiceProvider destructor
 */
WiFiServiceProvider::~WiFiServiceProvider(){
  this->m_wifi = nullptr;
}

/**
 * begin wifi functionality
 */
void WiFiServiceProvider::begin( iWiFiInterface* _wifi ){

  if ( nullptr == _wifi ) {
    return;
  }

  this->m_wifi = _wifi;
  wifi_config_table _wifi_credentials = __database_service.get_wifi_config_table();

  // this->m_wifi->mode(WIFI_AP_STA);
  this->m_wifi->setSleepMode(WIFI_NONE_SLEEP);  // WIFI_NONE_SLEEP = 0, WIFI_LIGHT_SLEEP = 1, WIFI_MODEM_SLEEP = 2
  this->m_wifi->setOutputPower(21.0);  // dBm max: +20.5dBm  min: 0dBm
  this->m_wifi->persistent(false);
  this->m_wifi->setAutoReconnect(false);
  this->configure_wifi_station( &_wifi_credentials );
  this->configure_wifi_access_point( &_wifi_credentials );
  __task_scheduler.setInterval( [&]() {
    this->handleWiFiConnectivity();
    #ifdef ENABLE_DYNAMIC_SUBNETTING
    this->reconfigure_wifi_access_point();
    #endif
    this->handleInternetConnectivity();
  }, WIFI_CONNECTIVITY_CHECK_DURATION );

  // __task_scheduler.setInterval( [&]() {
  //   this->handleInternetConnectivity();
  // }, INTERNET_CONNECTIVITY_CHECK_DURATION );
  _ClearObject(&_wifi_credentials);
}

/**
 * handle internet availability by ping function
 */
void WiFiServiceProvider::handleInternetConnectivity(){

  if ( nullptr == this->m_wifi ) {
    return;
  }

  #ifdef EW_SERIAL_LOG
  Log( F("\nhandeling internet connectivity :") );
  #endif

  if( !this->m_wifi->localIP().isSet() || !this->m_wifi->isConnected() ){

    memset( &__status_wifi, 0, sizeof(__status_wifi_t) );
    __status_wifi.last_internet_millis = millis();
  }else{

    bool ping_ret = __ping_service.ping();
    bool ping_resp = __ping_service.isHostRespondingToPing();

    if( ping_ret && ping_resp ){

      __status_wifi.last_internet_millis = millis();
    }
    __status_wifi.internet_available = ping_ret && ping_resp;

    #ifdef ENABLE_INTERNET_BASED_CONNECTIONS
    if( !__status_wifi.internet_available && (millis()-__status_wifi.last_internet_millis) >= SWITCHING_DURATION_FOR_NO_INTERNET_CONNECTION ){

      memcpy( __status_wifi.ignore_bssid, this->m_wifi->BSSID(), 6 );
      this->m_wifi->disconnect(false);
      __status_wifi.last_internet_millis = millis();

      __task_scheduler.setTimeout( [&]() {
        this->m_wifi->scanNetworksAsync( [&](int _scanCount) {
          this->scan_aps_and_configure_wifi_station_async(_scanCount);
        }, false);
      }, 500 );

      __task_scheduler.setTimeout( [&]() {
        #ifdef EW_SERIAL_LOG
        Logln( F("\nhandle station reconnecting...") );
        #endif
        memset( __status_wifi.ignore_bssid, 0, 6 );
        if( !this->m_wifi->localIP().isSet() || !this->m_wifi->isConnected() ){
          wifi_config_table _wifi_credentials = __database_service.get_wifi_config_table();
          this->configure_wifi_station( &_wifi_credentials );
          _ClearObject(&_wifi_credentials);
        }
      }, 2*INTERNET_CONNECTIVITY_CHECK_DURATION );
    }
    #endif
  }

  #ifdef EW_SERIAL_LOG
  Log( __status_wifi.internet_available ); Log( F(" : ") );
  Logln( (millis()-__status_wifi.last_internet_millis) );
  #endif
}

/**
 * return station subnet ip address
 *
 * @return  uint32_t
 */
uint32_t WiFiServiceProvider::getStationSubnetIP(void){

  uint32_t _subnet_ip4 = 0;
  if( nullptr != this->m_wifi && this->m_wifi->isConnected() && this->m_wifi->localIP().isSet() ){

    _subnet_ip4 = (uint32_t)this->m_wifi->subnetMask()&(uint32_t)this->m_wifi->localIP();
  }
  return _subnet_ip4;
}

/**
 * return station broadcast ip address
 *
 * @return  uint32_t
 */
uint32_t WiFiServiceProvider::getStationBroadcastIP(void){

  uint32_t _broadcast_ip4 = this->getStationSubnetIP();
  if( _broadcast_ip4 && nullptr != this->m_wifi ){

    uint32_t _no_of_ips = ~(uint32_t)this->m_wifi->subnetMask();
    _broadcast_ip4 += (_no_of_ips - 1);
  }
  return _broadcast_ip4;
}

/**
 * configure and start wifi station functionality
 *
 * @param   wifi_config_table* _wifi_credentials
 * @return  bool
 */
bool WiFiServiceProvider::configure_wifi_station( wifi_config_table* _wifi_credentials, uint8_t* mac ){

  if( nullptr == this->m_wifi || nullptr == _wifi_credentials ){
    return false;
  }

  #ifdef EW_SERIAL_LOG
    Log(F("\nConnecing To "));
    Log(_wifi_credentials->sta_ssid);
    if( nullptr != mac ){
      Log(F(" : "));
      for (uint8_t i = 0; i < 6; i++) {
        Log_format(mac[i], HEX);
      }
    }
  #endif

  IPAddress local_IP(
    _wifi_credentials->sta_local_ip[0],_wifi_credentials->sta_local_ip[1],_wifi_credentials->sta_local_ip[2],_wifi_credentials->sta_local_ip[3]
  );
  IPAddress gateway(
    _wifi_credentials->sta_gateway[0],_wifi_credentials->sta_gateway[1],_wifi_credentials->sta_gateway[2],_wifi_credentials->sta_gateway[3]
  );
  IPAddress subnet(
    _wifi_credentials->sta_subnet[0],_wifi_credentials->sta_subnet[1],_wifi_credentials->sta_subnet[2],_wifi_credentials->sta_subnet[3]
  );

  this->m_wifi->enableSTA(true);
  this->m_wifi->config( local_IP, gateway, subnet );
  this->m_wifi->begin(_wifi_credentials->sta_ssid, _wifi_credentials->sta_password, 0, mac);

  uint8_t _wait = 1;
  while ( ! this->m_wifi->isConnected() ) {

    delay(999);
    if( _wait%7 == 0 ){
      #ifdef EW_SERIAL_LOG
        Log(F("\ntrying reconnect"));
      #endif
      this->m_wifi->reconnect();
    }
    if( _wait++ > this->m_wifi_connection_timeout ){
      break;
    }
    #ifdef EW_SERIAL_LOG
      Log(".");
    #endif
  }
  #ifdef EW_SERIAL_LOG
  Logln();
  #endif
  if( this->m_wifi->status() == WL_CONNECTED ){
    #ifdef EW_SERIAL_LOG
    Log(F("Connected to "));
    Logln(_wifi_credentials->sta_ssid);
    Log(F("IP address: "));
    Logln(this->m_wifi->localIP());
    #endif
    // this->m_wifi->setAutoConnect(true);
    // this->m_wifi->setAutoReconnect(true);
    return true;
  }else if( this->m_wifi->status() == WL_NO_SSID_AVAIL ){
    #ifdef EW_SERIAL_LOG
    Log(_wifi_credentials->sta_ssid);
    Logln(F(" Not Found/reachable. Make sure it's availability."));
    #endif
  }else if( this->m_wifi->status() == WL_CONNECT_FAILED ){
    #ifdef EW_SERIAL_LOG
    Log(_wifi_credentials->sta_ssid);
    Logln(F(" is available but not connecting. Please check password."));
    #endif
  }else{
    #ifdef EW_SERIAL_LOG
      Logln(F("WiFi Not Connecting. Will try later soon.."));
    #endif
  }
  return false;
}

#ifdef ENABLE_DYNAMIC_SUBNETTING
/**
 * reconfigure and start wifi access point functionality
 *
 */
void WiFiServiceProvider::reconfigure_wifi_access_point( void ){

  if( nullptr == this->m_wifi ){
    return;
  }

  #ifdef EW_SERIAL_LOG
  Logln( F("Handeling reconfigure AP.") );
  #endif
  wifi_config_table _wifi_credentials = __database_service.get_wifi_config_table();
  bool _ap_change = false;

  if( !this->m_wifi->localIP().isSet() || !this->m_wifi->isConnected() ){

    // if( __are_arrays_equal( (char*)_wifi_credentials.ap_local_ip, (char*)_wifi_config_defaults.ap_local_ip, 4 ) ){
    if(
      _wifi_credentials.ap_local_ip[0] != _wifi_config_defaults.ap_local_ip[0] ||
      _wifi_credentials.ap_local_ip[1] != _wifi_config_defaults.ap_local_ip[1] ||
      _wifi_credentials.ap_local_ip[2] != _wifi_config_defaults.ap_local_ip[2] ||
      _wifi_credentials.ap_local_ip[3] != _wifi_config_defaults.ap_local_ip[3]
    ){

      memcpy_P( _wifi_credentials.ap_local_ip, _wifi_config_defaults.ap_local_ip, 4);
      memcpy_P( _wifi_credentials.ap_gateway, _wifi_config_defaults.ap_gateway, 4);

      __database_service.set_wifi_config_table(&_wifi_credentials);
      _ap_change = true;
    }
  }else{

    IPAddress gateway_IP = this->m_wifi->gatewayIP();
    IPAddress sta_subnet_ip = IPAddress(this->getStationSubnetIP());

    if(
      (( gateway_IP[3] - sta_subnet_ip[3] ) == 1 &&
      ( sta_subnet_ip[0] == _wifi_config_defaults.ap_gateway[0] ) &&
      ( sta_subnet_ip[1] == _wifi_config_defaults.ap_gateway[1] )) &&
      (( _wifi_credentials.ap_gateway[2] - sta_subnet_ip[2] ) != 1 ||
      ( _wifi_credentials.ap_gateway[3] != sta_subnet_ip[3]+1 ))
    ){

      _wifi_credentials.ap_local_ip[0] = sta_subnet_ip[0];
      _wifi_credentials.ap_local_ip[1] = sta_subnet_ip[1];
      _wifi_credentials.ap_local_ip[2] = (sta_subnet_ip[2]+1) < 255 ? (sta_subnet_ip[2]+1) : _wifi_config_defaults.ap_local_ip[2];
      _wifi_credentials.ap_local_ip[3] = sta_subnet_ip[3]+1;

      memcpy( _wifi_credentials.ap_gateway, _wifi_credentials.ap_local_ip, 4 );

      __database_service.set_wifi_config_table(&_wifi_credentials);
      _ap_change = true;
    }
  }

  if( _ap_change ){
    #ifdef EW_SERIAL_LOG
    Logln( F("reconfiguring....") );
    #endif
    this->m_wifi->softAPdisconnect(false);
    // this->m_wifi->enableAP(false);

    __task_scheduler.setTimeout( [&](){
      wifi_config_table __wifi_credentials = __database_service.get_wifi_config_table();
      this->configure_wifi_access_point(&__wifi_credentials);
      _ClearObject(&__wifi_credentials);
    }, 1);
  }
  _ClearObject(&_wifi_credentials);
}
#endif

/**
 * configure and start wifi access point functionality
 *
 * @param   wifi_config_table* _wifi_credentials
 * @return  bool
 */
bool WiFiServiceProvider::configure_wifi_access_point( wifi_config_table* _wifi_credentials ){

  if( nullptr == this->m_wifi || nullptr == _wifi_credentials ){
    return false;
  }

  #ifdef EW_SERIAL_LOG
  Log(F("Configuring access point "));
  Log( _wifi_credentials->ap_ssid );
  Logln(F(" .."));
  #endif

  IPAddress local_IP(
    _wifi_credentials->ap_local_ip[0],_wifi_credentials->ap_local_ip[1],_wifi_credentials->ap_local_ip[2],_wifi_credentials->ap_local_ip[3]
  );
  IPAddress gateway(
    _wifi_credentials->ap_gateway[0],_wifi_credentials->ap_gateway[1],_wifi_credentials->ap_gateway[2],_wifi_credentials->ap_gateway[3]
  );
  IPAddress subnet(
    _wifi_credentials->ap_subnet[0],_wifi_credentials->ap_subnet[1],_wifi_credentials->ap_subnet[2],_wifi_credentials->ap_subnet[3]
  );

  this->m_wifi->enableAP(true);
  if( this->m_wifi->softAPConfig( local_IP, gateway, subnet ) &&
    this->m_wifi->softAP( _wifi_credentials->ap_ssid, _wifi_credentials->ap_password, 1, 0, 8 )
  ){
    #ifdef EW_SERIAL_LOG
    Log(F("AP IP address: "));
    Logln(this->m_wifi->softAPIP());
    #endif
    return true;
  }else{
    #ifdef EW_SERIAL_LOG
    Logln(F("Configuring access point failed!"));
    #endif
    return false;
  }
}

/**
 * scan ssid in connected stations and return bssid and found result
 *
 * @param   char*     ssid
 * @param   uint8_t*  bssid
 * @return  bool
 */
bool WiFiServiceProvider::scan_within_station_async( char* ssid, uint8_t* bssid, int _scanCount ){

  if( nullptr == this->m_wifi || nullptr == ssid || nullptr == bssid ){
    return false;
  }

  #ifdef EW_SERIAL_LOG
    Plain_Logln(F("Scanning stations"));
  #endif

  int n = _scanCount;
  // int indices[n];
  // for (int i = 0; i < n; i++) {
  //   indices[i] = i;
  // }
  // for (int i = 0; i < n; i++) {
  //   for (int j = i + 1; j < n; j++) {
  //     if (this->m_wifi->RSSI(indices[j]) > this->m_wifi->RSSI(indices[i])) {
  //       std::swap(indices[i], indices[j]);
  //     }
  //   }
  // }
  struct station_info * stat_info = wifi_softap_get_station_info();
  struct station_info * stat_info_copy = stat_info;
  char* _ssid_buff = new char[WIFI_CONFIGS_BUF_SIZE];

  if( nullptr == _ssid_buff ){
    return false;
  }
  memset( _ssid_buff, 0, WIFI_CONFIGS_BUF_SIZE );

  for (int i = 0; i < n; ++i) {

    String _ssid = this->m_wifi->SSID(i);
    _ssid.toCharArray( _ssid_buff, _ssid.length()+1 );

    // #ifdef EW_SERIAL_LOG
    //   Plain_Log(_ssid_buff);
    //   Plain_Log(F(" : "));
    //   Plain_Logln(this->m_wifi->BSSIDstr(i));
    // #endif

    if( __are_arrays_equal( ssid, _ssid_buff, _ssid.length()+1 ) ){

      bool _found = false;
      stat_info = stat_info_copy;
      while ( nullptr != stat_info ) {

        memcpy(bssid, stat_info->bssid, 6);
        bssid[0] +=2;

        if( __are_arrays_equal( (char*)bssid, (char*)this->m_wifi->BSSID(i), 6 ) ){

          _found = true;
          break;
        }

        stat_info = STAILQ_NEXT(stat_info, next);
      }

      if( !_found ){

        if( !__are_arrays_equal( (char*)__status_wifi.ignore_bssid, (char*)this->m_wifi->BSSID(i), 6 ) ){
          memcpy(bssid, this->m_wifi->BSSID(i), 6);
          delete[] _ssid_buff;
          return true;
        }
      }
    }
  }

  delete[] _ssid_buff;
  return false;
}

/**
 * scan ssid in connected stations and return bssid and found result
 *
 * @param   char*     ssid
 * @param   uint8_t*  bssid
 * @return  bool
 */
bool WiFiServiceProvider::scan_within_station( char* ssid, uint8_t* bssid ){

  if( nullptr == this->m_wifi || nullptr == ssid || nullptr == bssid ){
    return false;
  }
  int n = this->m_wifi->scanNetworks();
  return scan_within_station_async( ssid, bssid, n );
}

/**
 * scan connected stations then configure and start wifi station functionality
 *
 * @param   int _scanCount
 */
void WiFiServiceProvider::scan_aps_and_configure_wifi_station_async( int _scanCount ){

  wifi_config_table _wifi_credentials = __database_service.get_wifi_config_table();
  if( this->scan_within_station_async( _wifi_credentials.sta_ssid, this->m_temp_mac, _scanCount ) ) {
    __task_scheduler.setTimeout([&](){
      wifi_config_table __wifi_credentials = __database_service.get_wifi_config_table();
      this->configure_wifi_station( &__wifi_credentials, this->m_temp_mac );
      _ClearObject(&__wifi_credentials);
    }, 1);
  }
  _ClearObject(&_wifi_credentials);
}

/**
 * scan connected stations then configure and start wifi station functionality
 *
 */
void WiFiServiceProvider::scan_aps_and_configure_wifi_station( ){

  // #ifdef EW_SERIAL_LOG
  //   Logln(F("scanning connected stations for wifi config.."));
  // #endif
  wifi_config_table _wifi_credentials = __database_service.get_wifi_config_table();
  if( this->scan_within_station( _wifi_credentials.sta_ssid, this->m_temp_mac) ){
    __task_scheduler.setTimeout([&](){
      wifi_config_table __wifi_credentials = __database_service.get_wifi_config_table();
      this->configure_wifi_station( &__wifi_credentials, this->m_temp_mac );
      _ClearObject(&__wifi_credentials);
    }, 1);
  }
  _ClearObject(&_wifi_credentials);
}

/**
 * check wifi connectivity after each wifi activity cycle. try to reconnect if failed
 */
void WiFiServiceProvider::handleWiFiConnectivity(){

  if( nullptr == this->m_wifi ){
    return;
  }

  #ifdef EW_SERIAL_LOG
  Logln( F("\nHandeling WiFi Connectivity") );
  #endif

  if( !this->m_wifi->localIP().isSet() || !this->m_wifi->isConnected() ){

    #ifdef EW_SERIAL_LOG
    Log( F("Handeling WiFi Reconnect Manually : ") );
    Logln(this->m_wifi->softAPIP());
    // Log(F(" : "));
    // Logln(number_client);
    #endif

    #ifdef IGNORE_FREE_RELAY_CONNECTIONS
    this->m_wifi->reconnect();
    #else
    uint8_t number_client= wifi_softap_get_station_num();

    if( number_client > 0 ){
      this->m_wifi->scanNetworksAsync( [&](int _scanCount) {
        this->scan_aps_and_configure_wifi_station_async(_scanCount);
      }, false);
    }else{
      this->m_wifi->reconnect();
    }
    #endif
    __status_wifi.wifi_connected = false;
  }else{
    __status_wifi.wifi_connected = true;
    #ifdef EW_SERIAL_LOG
    Log(F("IP address: "));
    Log(this->m_wifi->gatewayIP());
    Log(F(" : "));
    Log(this->m_wifi->localIP());
    Log(F(" : "));
    Logln(this->m_wifi->softAPIP());
    #endif
  }
}

#ifdef EW_SERIAL_LOG
/**
 * print wifi configs
 */
void WiFiServiceProvider::printWiFiConfigLogs(){

  wifi_config_table _table = __database_service.get_wifi_config_table();
  char _ip_address[20];

  Logln(F("\nWiFi Configs :"));
  // Logln(F("ssid\tpassword\tlocal\tgateway\tsubnet"));

  Log(_table.sta_ssid); Log("\t");
  Log(_table.sta_password); Log("\t");
  __int_ip_to_str( _ip_address, _table.sta_local_ip, 20 ); Log(_ip_address); Log("\t");
  __int_ip_to_str( _ip_address, _table.sta_gateway, 20 ); Log(_ip_address); Log("\t");
  __int_ip_to_str( _ip_address, _table.sta_subnet, 20 ); Log(_ip_address); Log("\t\n");

  Logln(F("\nAccess Configs :"));
  // Logln(F("ssid\tpassword\tlocal\tgateway\tsubnet"));
  Log(_table.ap_ssid); Log("\t");
  Log(_table.ap_password); Log("\t");
  __int_ip_to_str( _ip_address, _table.ap_local_ip, 20 ); Log(_ip_address); Log("\t");
  __int_ip_to_str( _ip_address, _table.ap_gateway, 20 ); Log(_ip_address); Log("\t");
  __int_ip_to_str( _ip_address, _table.ap_subnet, 20 ); Log(_ip_address); Log("\t\n\n");
}
#endif

WiFiServiceProvider __wifi_service;
