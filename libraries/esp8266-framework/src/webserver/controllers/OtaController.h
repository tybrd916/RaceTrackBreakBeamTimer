/******************************* OTA Controller *******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_OTA_CONTROLLER_
#define _EW_SERVER_OTA_CONTROLLER_

#include "Controller.h"
#include <webserver/pages/WiFiConfigPage.h>
#include <webserver/pages/OtaConfigPage.h>

/**
 * OtaController class
 */
class OtaController : public Controller {

	public:

		/**
		 * OtaController constructor
		 */
		OtaController():Controller("ota"){
		}

		/**
		 * OtaController destructor
		 */
		~OtaController(){
		}

		/**
		 * register ota controller
		 *
		 */
		void boot( void ){

			if( nullptr != this->m_route_handler ){
				this->m_route_handler->register_route( EW_SERVER_OTA_CONFIG_ROUTE, [&]() { this->handleOtaServerConfigRoute(); }, AUTH_MIDDLEWARE );
			}
		}

		/**
		 * build ota server config html.
		 *
		 * @param	char*	_page
		 * @param	bool|false	_enable_flash
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
		void build_ota_server_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ){
				return;
			}

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_OTA_CONFIG_PAGE_TOP );

      ota_config_table _ota_configs = this->m_web_resource->m_db_conn->get_ota_config_table();

      char _port[10];memset( _port, 0, 10 );
      __appendUintToBuff( _port, "%d", _ota_configs.ota_port, 8 );

			#ifdef ALLOW_OTA_CONFIG_MODIFICATION

			concat_tr_input_html_tags( _page, PSTR("OTA Host:"), PSTR("hst"), _ota_configs.ota_host, OTA_HOST_BUF_SIZE-1 );
      concat_tr_input_html_tags( _page, PSTR("OTA Port:"), PSTR("prt"), _port );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );

			#else

			concat_tr_input_html_tags( _page, PSTR("OTA Host:"), PSTR("hst"), _ota_configs.ota_host, OTA_HOST_BUF_SIZE-1, HTML_INPUT_TEXT_TAG_TYPE, false, true );
      concat_tr_input_html_tags( _page, PSTR("OTA Port:"), PSTR("prt"), _port, HTML_INPUT_TAG_DEFAULT_MAXLENGTH, HTML_INPUT_TEXT_TAG_TYPE, false, true );

			#endif

      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
    }

		/**
		 * build and send ota server config page.
		 * when posted, get ota server configs from client and set them in database.
		 */
    void handleOtaServerConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling OTA Server Config route"));
      #endif

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ||
					nullptr == this->m_web_resource->m_server ){
				return;
			}

      bool _is_posted = false;

			#ifdef ALLOW_OTA_CONFIG_MODIFICATION
      if ( this->m_web_resource->m_server->hasArg("hst") && this->m_web_resource->m_server->hasArg("prt") ) {

        String _ota_host = this->m_web_resource->m_server->arg("hst");
        String _ota_port = this->m_web_resource->m_server->arg("prt");

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
          Log(F("ota host : ")); Logln( _ota_host );
          Log(F("ota port : ")); Logln( _ota_port );
          Logln();
        #endif

        ota_config_table _ota_configs = this->m_web_resource->m_db_conn->get_ota_config_table();

        _ota_host.toCharArray( _ota_configs.ota_host, _ota_host.length()+1 );
        _ota_configs.ota_port = (int)_ota_port.toInt();

        this->m_web_resource->m_db_conn->set_ota_config_table( &_ota_configs);

        _is_posted = true;
      }
			#endif

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_ota_server_config_html( _page, _is_posted );

      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

};

#endif
