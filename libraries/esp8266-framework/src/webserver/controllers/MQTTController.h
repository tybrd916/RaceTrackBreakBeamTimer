/******************************* MQTT Controller ******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_MQTT_CONTROLLER_
#define _EW_SERVER_MQTT_CONTROLLER_

#include "Controller.h"
#include <webserver/pages/MqttConfigPage.h>
#include <service_provider/MqttServiceProvider.h>

/**
 * MqttController class
 */
class MqttController : public Controller {

	public:

		/**
		 * MqttController constructor
		 */
		MqttController():Controller("mqtt"){
		}

		/**
		 * MqttController destructor
		 */
		~MqttController(){
		}

		/**
		 * register mqtt controller
		 *
		 */
		void boot( void ){

			if( nullptr != this->m_route_handler ){
				this->m_route_handler->register_route( EW_SERVER_MQTT_MANAGE_CONFIG_ROUTE, [&]() { this->handleMqttManageRoute(); }, AUTH_MIDDLEWARE );
	      this->m_route_handler->register_route( EW_SERVER_MQTT_GENERAL_CONFIG_ROUTE, [&]() { this->handleMqttGeneralConfigRoute(); }, AUTH_MIDDLEWARE );
	      this->m_route_handler->register_route( EW_SERVER_MQTT_LWT_CONFIG_ROUTE, [&]() { this->handleMqttLWTConfigRoute(); }, AUTH_MIDDLEWARE );
	      this->m_route_handler->register_route( EW_SERVER_MQTT_PUBSUB_CONFIG_ROUTE, [&]() { this->handleMqttPubSubConfigRoute(); }, AUTH_MIDDLEWARE );
			}
		}

		/**
		 * build and send mqtt manage page to client
		 */
    void handleMqttManageRoute( void ) {

      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Mqtt Manage route"));
      #endif

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_server ){
				return;
			}

      char* _page = new char[EW_HTML_MAX_SIZE];
			memset( _page, 0, EW_HTML_MAX_SIZE );

			strcat_P( _page, EW_SERVER_HEADER_HTML );
			strcat_P( _page, EW_SERVER_MENU_CARD_PAGE_WRAP_TOP );

			concat_svg_menu_card( _page, EW_SERVER_MQTT_MENU_TITLE_GENERAL, SVG_ICON48_PATH_SETTINGS, EW_SERVER_MQTT_GENERAL_CONFIG_ROUTE );
			concat_svg_menu_card( _page, EW_SERVER_MQTT_MENU_TITLE_LWT, SVG_ICON48_PATH_BEENHERE, EW_SERVER_MQTT_LWT_CONFIG_ROUTE );
			concat_svg_menu_card( _page, EW_SERVER_MQTT_MENU_TITLE_PUBSUB, SVG_ICON48_PATH_IMPORT_EXPORT, EW_SERVER_MQTT_PUBSUB_CONFIG_ROUTE );

			strcat_P( _page, EW_SERVER_MENU_CARD_PAGE_WRAP_BOTTOM );
			strcat_P( _page, EW_SERVER_FOOTER_HTML );

      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

		/**
		 * build mqtt general config html.
		 *
		 * @param	char*	_page
		 * @param	bool|false	_enable_flash
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
    void build_mqtt_general_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ){
				return;
			}

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_MQTT_GENERAL_PAGE_TOP );

      mqtt_general_config_table _mqtt_general_configs = this->m_web_resource->m_db_conn->get_mqtt_general_config_table();

      char _port[10],_keepalive[10];memset( _port, 0, 10 );memset( _keepalive, 0, 10 );
      __appendUintToBuff( _port, "%d", _mqtt_general_configs.port, 8 );
      __appendUintToBuff( _keepalive, "%d", _mqtt_general_configs.keepalive, 8 );

			#ifdef ALLOW_MQTT_CONFIG_MODIFICATION

      concat_tr_input_html_tags( _page, PSTR("Host Address:"), PSTR("hst"), _mqtt_general_configs.host, MQTT_HOST_BUF_SIZE-1 );
      concat_tr_input_html_tags( _page, PSTR("Host Port:"), PSTR("prt"), _port );
      concat_tr_input_html_tags( _page, PSTR("Client Id:"), PSTR("clid"), _mqtt_general_configs.client_id, MQTT_CLIENT_ID_BUF_SIZE-1 );
      concat_tr_input_html_tags( _page, PSTR("Username:"), PSTR("usrn"), _mqtt_general_configs.username, MQTT_USERNAME_BUF_SIZE-1 );
      concat_tr_input_html_tags( _page, PSTR("Password:"), PSTR("pswd"), _mqtt_general_configs.password, MQTT_PASSWORD_BUF_SIZE-1 );
      concat_tr_input_html_tags( _page, PSTR("Keep Alive:"), PSTR("kpalv"), _keepalive );
      concat_tr_input_html_tags( _page, PSTR("Clean Session:"), PSTR("cln"), "clean", HTML_INPUT_TAG_DEFAULT_MAXLENGTH, HTML_INPUT_CHECKBOX_TAG_TYPE,_mqtt_general_configs.clean_session != 0 );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );

			#else

			concat_tr_input_html_tags( _page, PSTR("Host Address:"), PSTR("hst"), _mqtt_general_configs.host, MQTT_HOST_BUF_SIZE-1, HTML_INPUT_TEXT_TAG_TYPE, false, true );
      concat_tr_input_html_tags( _page, PSTR("Host Port:"), PSTR("prt"), _port, HTML_INPUT_TAG_DEFAULT_MAXLENGTH, HTML_INPUT_TEXT_TAG_TYPE, false, true );
      concat_tr_input_html_tags( _page, PSTR("Client Id:"), PSTR("clid"), _mqtt_general_configs.client_id, MQTT_CLIENT_ID_BUF_SIZE-1, HTML_INPUT_TEXT_TAG_TYPE, false, true );
      concat_tr_input_html_tags( _page, PSTR("Username:"), PSTR("usrn"), _mqtt_general_configs.username, MQTT_USERNAME_BUF_SIZE-1, HTML_INPUT_TEXT_TAG_TYPE, false, true );
      concat_tr_input_html_tags( _page, PSTR("Password:"), PSTR("pswd"), _mqtt_general_configs.password, MQTT_PASSWORD_BUF_SIZE-1, HTML_INPUT_TEXT_TAG_TYPE, false, true );
      concat_tr_input_html_tags( _page, PSTR("Keep Alive:"), PSTR("kpalv"), _keepalive, HTML_INPUT_TAG_DEFAULT_MAXLENGTH, HTML_INPUT_TEXT_TAG_TYPE, false, true );
      concat_tr_input_html_tags( _page, PSTR("Clean Session:"), PSTR("cln"), "clean", HTML_INPUT_TAG_DEFAULT_MAXLENGTH, HTML_INPUT_CHECKBOX_TAG_TYPE,_mqtt_general_configs.clean_session != 0, true );

			#endif

      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
    }

		/**
		 * build and send mqtt general config page.
		 * when posted, get mqtt general configs from client and set them in database.
		 */
    void handleMqttGeneralConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Mqtt General Config route"));
      #endif

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ||
					nullptr == this->m_web_resource->m_server ){
				return;
			}

      bool _is_posted = false;

			#ifdef ALLOW_MQTT_CONFIG_MODIFICATION
      if ( this->m_web_resource->m_server->hasArg("hst") && this->m_web_resource->m_server->hasArg("prt") ) {

        String _mqtt_host = this->m_web_resource->m_server->arg("hst");
        String _mqtt_port = this->m_web_resource->m_server->arg("prt");
        String _client_id = this->m_web_resource->m_server->arg("clid");
        String _username = this->m_web_resource->m_server->arg("usrn");
        String _password = this->m_web_resource->m_server->arg("pswd");
        String _keep_alive = this->m_web_resource->m_server->arg("kpalv");
        String _clean_session = this->m_web_resource->m_server->arg("cln");

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
          Log(F("mqtt host : ")); Logln( _mqtt_host );
          Log(F("mqtt port : ")); Logln( _mqtt_port );
          Log(F("client id : ")); Logln( _client_id );
          Log(F("username : ")); Logln( _username );
          Log(F("password : ")); Logln( _password );
          Log(F("keep alive : ")); Logln( _keep_alive );
          Log(F("clean session : ")); Logln( _clean_session );
          Logln();
        #endif

        mqtt_general_config_table* _mqtt_general_configs = new mqtt_general_config_table;
        memset( (void*)_mqtt_general_configs, 0, sizeof(mqtt_general_config_table) );
        // mqtt_general_config_table _mqtt_general_configs = this->m_db_conn->get_mqtt_general_config_table();

        _mqtt_host.toCharArray( _mqtt_general_configs->host, _mqtt_host.length()+1 );
        _mqtt_general_configs->port = (int)_mqtt_port.toInt();
        _client_id.toCharArray( _mqtt_general_configs->client_id, _client_id.length()+1 );
        _username.toCharArray( _mqtt_general_configs->username, _username.length()+1 );
        _password.toCharArray( _mqtt_general_configs->password, _password.length()+1 );
        _mqtt_general_configs->keepalive = (int)_keep_alive.toInt();
        _mqtt_general_configs->clean_session = (int)( _clean_session == "clean" );

        this->m_web_resource->m_db_conn->set_mqtt_general_config_table( _mqtt_general_configs);
        delete _mqtt_general_configs;

        _is_posted = true;
      }
			#endif

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_mqtt_general_config_html( _page, _is_posted );

      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
			if( _is_posted ){
        __mqtt_service.handleMqttConfigChange(MQTT_GENERAL_CONFIG);
      }
    }

		/**
		 * build mqtt lwt config html.
		 *
		 * @param	char*	_page
		 * @param	bool|false	_enable_flash
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
    void build_mqtt_lwt_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ){
				return;
			}

      memset( _page, 0, _max_size );
      char _ip_address[20];
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_MQTT_LWT_PAGE_TOP );

      mqtt_lwt_config_table _mqtt_lwt_configs = this->m_web_resource->m_db_conn->get_mqtt_lwt_config_table();

      char* _qos_options[] = {"0", "1", "2"};

			#ifdef ALLOW_MQTT_CONFIG_MODIFICATION

			concat_tr_input_html_tags( _page, PSTR("Will Topic:"), PSTR("wtpc"), _mqtt_lwt_configs.will_topic, MQTT_TOPIC_BUF_SIZE-1 );
      concat_tr_input_html_tags( _page, PSTR("Will Message:"), PSTR("wmsg"), _mqtt_lwt_configs.will_message, MQTT_TOPIC_BUF_SIZE-1 );
      concat_tr_select_html_tags( _page, PSTR("Will QoS:"), PSTR("wqos"), _qos_options, 3, _mqtt_lwt_configs.will_qos );
      concat_tr_input_html_tags( _page, PSTR("Will Retain:"), PSTR("wrtn"), "retain", HTML_INPUT_TAG_DEFAULT_MAXLENGTH, HTML_INPUT_CHECKBOX_TAG_TYPE,_mqtt_lwt_configs.will_retain != 0 );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );

			#else

			concat_tr_input_html_tags( _page, PSTR("Will Topic:"), PSTR("wtpc"), _mqtt_lwt_configs.will_topic, MQTT_TOPIC_BUF_SIZE-1, HTML_INPUT_TEXT_TAG_TYPE, false, true );
      concat_tr_input_html_tags( _page, PSTR("Will Message:"), PSTR("wmsg"), _mqtt_lwt_configs.will_message, MQTT_TOPIC_BUF_SIZE-1, HTML_INPUT_TEXT_TAG_TYPE, false, true );
      concat_tr_select_html_tags( _page, PSTR("Will QoS:"), PSTR("wqos"), _qos_options, 3, _mqtt_lwt_configs.will_qos, 0, true );
      concat_tr_input_html_tags( _page, PSTR("Will Retain:"), PSTR("wrtn"), "retain", HTML_INPUT_TAG_DEFAULT_MAXLENGTH, HTML_INPUT_CHECKBOX_TAG_TYPE,_mqtt_lwt_configs.will_retain != 0, true );

			#endif

      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
    }

		/**
		 * build and send mqtt lwt config page.
		 * when posted, get mqtt lwt configs from client and set them in database.
		 */
    void handleMqttLWTConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Mqtt LWT Config route"));
      #endif

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ||
					nullptr == this->m_web_resource->m_server ){
				return;
			}

      bool _is_posted = false;

			#ifdef ALLOW_MQTT_CONFIG_MODIFICATION
      if ( this->m_web_resource->m_server->hasArg("wtpc") && this->m_web_resource->m_server->hasArg("wmsg") ) {

        String _will_topic = this->m_web_resource->m_server->arg("wtpc");
        String _will_message = this->m_web_resource->m_server->arg("wmsg");
        String _will_qos = this->m_web_resource->m_server->arg("wqos");
        String _will_retain = this->m_web_resource->m_server->arg("wrtn");

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
          Log(F("will topic : ")); Logln( _will_topic );
          Log(F("will message : ")); Logln( _will_message );
          Log(F("will qos : ")); Logln( _will_qos );
          Log(F("will retain : ")); Logln( _will_retain );
          Logln();
        #endif

        mqtt_lwt_config_table* _mqtt_lwt_configs = new mqtt_lwt_config_table;
        memset( (void*)_mqtt_lwt_configs, 0, sizeof(mqtt_lwt_config_table) );

        _will_topic.toCharArray( _mqtt_lwt_configs->will_topic, _will_topic.length()+1 );
        _will_message.toCharArray( _mqtt_lwt_configs->will_message, _will_message.length()+1 );
        _mqtt_lwt_configs->will_qos = (int)_will_qos.toInt() ;
        _mqtt_lwt_configs->will_retain = (int)( _will_retain == "retain" );

        this->m_web_resource->m_db_conn->set_mqtt_lwt_config_table( _mqtt_lwt_configs );

        delete _mqtt_lwt_configs;
        _is_posted = true;
      }
			#endif

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_mqtt_lwt_config_html( _page, _is_posted );

      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
			if( _is_posted ){
        __mqtt_service.handleMqttConfigChange(MQTT_LWT_CONFIG);
      }
    }

		/**
		 * build mqtt publish subscribe config html.
		 *
		 * @param	char*	_page
		 * @param	bool|false	_enable_flash
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
    void build_mqtt_pubsub_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ){
				return;
			}

      memset( _page, 0, _max_size );
      char _ip_address[20];
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_MQTT_PUBSUB_PAGE_TOP );

      mqtt_pubsub_config_table _mqtt_pubsub_configs = this->m_web_resource->m_db_conn->get_mqtt_pubsub_config_table();

      char* _qos_options[] = {"0", "1", "2"};
      char _topic_name[10], _topic_label[10];memset(_topic_name, 0, 10);memset(_topic_label, 0, 10);
      char _qos_name[10], _qos_label[10];memset(_qos_name, 0, 10);memset(_qos_label, 0, 10);
      char _retain_name[10], _retain_label[10];memset(_retain_name, 0, 10);memset(_retain_label, 0, 10);
      char _publish_freq[10]; memset(_publish_freq, 0, 10);__appendUintToBuff( _publish_freq, "%d", _mqtt_pubsub_configs.publish_frequency, 8 );
      strcpy( _topic_label, "Topic0:" );strcpy( _topic_name, "ptpc0" );
      strcpy( _qos_label, "QoS0:" );strcpy( _qos_name, "pqos0" );
      strcpy( _retain_label, "Retain0:" );strcpy( _retain_name, "prtn0" );

      concat_tr_heading_html_tags( _page, PSTR("Publish Topics"), 3, PSTR("2") );
      for (uint8_t i = 0; i < MQTT_MAX_PUBLISH_TOPIC; i++) {

        _topic_label[5] = (0x30 + i );_topic_name[4] = (0x30 + i );
        _qos_label[3] = (0x30 + i );_qos_name[4] = (0x30 + i );
        _retain_label[6] = (0x30 + i );_retain_name[4] = (0x30 + i );

				#ifdef ALLOW_MQTT_CONFIG_MODIFICATION

				concat_tr_input_html_tags( _page, _topic_label, _topic_name, _mqtt_pubsub_configs.publish_topics[i].topic, MQTT_TOPIC_BUF_SIZE-1 );
        concat_tr_select_html_tags( _page, _qos_label, _qos_name, _qos_options, 3, _mqtt_pubsub_configs.publish_topics[i].qos );
        concat_tr_input_html_tags( _page, _retain_label, _retain_name, "retain", HTML_INPUT_TAG_DEFAULT_MAXLENGTH, HTML_INPUT_CHECKBOX_TAG_TYPE, _mqtt_pubsub_configs.publish_topics[i].retain != 0 );

				#else

				concat_tr_input_html_tags( _page, _topic_label, _topic_name, _mqtt_pubsub_configs.publish_topics[i].topic, MQTT_TOPIC_BUF_SIZE-1, HTML_INPUT_TEXT_TAG_TYPE, false, true );
        concat_tr_select_html_tags( _page, _qos_label, _qos_name, _qos_options, 3, _mqtt_pubsub_configs.publish_topics[i].qos, 0, true );
        concat_tr_input_html_tags( _page, _retain_label, _retain_name, "retain", HTML_INPUT_TAG_DEFAULT_MAXLENGTH, HTML_INPUT_CHECKBOX_TAG_TYPE, _mqtt_pubsub_configs.publish_topics[i].retain != 0, true );

				#endif

      }
			#ifdef ALLOW_MQTT_CONFIG_MODIFICATION

			concat_tr_input_html_tags( _page, PSTR("Publish Frequency:"), PSTR("pfrq"), _publish_freq );

			#else

			concat_tr_input_html_tags( _page, PSTR("Publish Frequency:"), PSTR("pfrq"), _publish_freq, HTML_INPUT_TAG_DEFAULT_MAXLENGTH, HTML_INPUT_TEXT_TAG_TYPE, false, true );

			#endif

      _topic_name[0] = 's'; _qos_name[0] = 's';
      concat_tr_heading_html_tags( _page, PSTR("Subscribe Topics"), 3, PSTR("2") );
      for (uint8_t i = 0; i < MQTT_MAX_SUBSCRIBE_TOPIC; i++) {

        _topic_label[5] = (0x30 + i );_topic_name[4] = (0x30 + i );
        _qos_label[3] = (0x30 + i );_qos_name[4] = (0x30 + i );

				#ifdef ALLOW_MQTT_CONFIG_MODIFICATION

				concat_tr_input_html_tags( _page, _topic_label, _topic_name, _mqtt_pubsub_configs.subscribe_topics[i].topic, MQTT_TOPIC_BUF_SIZE-1 );
        concat_tr_select_html_tags( _page, _qos_label, _qos_name, _qos_options, 3, _mqtt_pubsub_configs.subscribe_topics[i].qos );

				#else

				concat_tr_input_html_tags( _page, _topic_label, _topic_name, _mqtt_pubsub_configs.subscribe_topics[i].topic, MQTT_TOPIC_BUF_SIZE-1, HTML_INPUT_TEXT_TAG_TYPE, false, true );
        concat_tr_select_html_tags( _page, _qos_label, _qos_name, _qos_options, 3, _mqtt_pubsub_configs.subscribe_topics[i].qos, 0, true );

				#endif
      }

			#ifdef ALLOW_MQTT_CONFIG_MODIFICATION
      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
			#endif

      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
    }

		/**
		 * build and send mqtt pubsub config page.
		 * when posted, get mqtt pubsub configs from client and set them in database.
		 */
    void handleMqttPubSubConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Mqtt LWT Config route"));
      #endif

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ||
					nullptr == this->m_web_resource->m_server ){
				return;
			}

      bool _is_posted = false;

			#ifdef ALLOW_MQTT_CONFIG_MODIFICATION
      if ( this->m_web_resource->m_server->hasArg("ptpc0") && this->m_web_resource->m_server->hasArg("pqos0") ) {

        mqtt_pubsub_config_table* _mqtt_pubsub_configs = new mqtt_pubsub_config_table;
        memset( (void*)_mqtt_pubsub_configs, 0, sizeof(mqtt_pubsub_config_table) );

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n\nPublish Topics"));
        #endif
        char _topic_name[10];memset(_topic_name, 0, 10);strcpy( _topic_name, "ptpc0" );
        char _qos_name[10];memset(_qos_name, 0, 10);strcpy( _qos_name, "pqos0" );
        char _retain_name[10];memset(_retain_name, 0, 10);strcpy( _retain_name, "prtn0" );

        for (uint8_t i = 0; i < MQTT_MAX_PUBLISH_TOPIC; i++) {

          _topic_name[4] = (0x30 + i );
          _qos_name[4] = (0x30 + i );
          _retain_name[4] = (0x30 + i );

          String _topic = this->m_web_resource->m_server->arg(_topic_name);
          String _qos = this->m_web_resource->m_server->arg(_qos_name);
          String _retain = this->m_web_resource->m_server->arg(_retain_name);

          _topic.toCharArray( _mqtt_pubsub_configs->publish_topics[i].topic, _topic.length()+1 );
          _mqtt_pubsub_configs->publish_topics[i].qos = (int)_qos.toInt() ;
          _mqtt_pubsub_configs->publish_topics[i].retain = (int)( _retain == "retain" );

          #ifdef EW_SERIAL_LOG
            Log(F("Topic")); Log(i); Log(F(" : ")); Logln(_topic);
            Log(F("QoS")); Log(i); Log(F(" : ")); Logln(_qos);
            Log(F("Retain")); Log(i); Log(F(" : ")); Logln(_retain);
          #endif
        }

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubscribe Topics"));
        #endif
        _topic_name[0] = 's'; _qos_name[0] = 's';
        for (uint8_t i = 0; i < MQTT_MAX_SUBSCRIBE_TOPIC; i++) {

          _topic_name[4] = (0x30 + i );
          _qos_name[4] = (0x30 + i );

          String _topic = this->m_web_resource->m_server->arg(_topic_name);
          String _qos = this->m_web_resource->m_server->arg(_qos_name);

          _topic.toCharArray( _mqtt_pubsub_configs->subscribe_topics[i].topic, _topic.length()+1 );
          _mqtt_pubsub_configs->subscribe_topics[i].qos = (int)_qos.toInt() ;

          #ifdef EW_SERIAL_LOG
            Log(F("Topic")); Log(i); Log(F(" : ")); Logln(_topic);
            Log(F("QoS")); Log(i); Log(F(" : ")); Logln(_qos);
          #endif
        }
        _mqtt_pubsub_configs->publish_frequency = (int)this->m_web_resource->m_server->arg("pfrq").toInt();

        this->m_web_resource->m_db_conn->set_mqtt_pubsub_config_table( _mqtt_pubsub_configs );

        delete _mqtt_pubsub_configs;
        _is_posted = true;
      }
			#endif

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_mqtt_pubsub_config_html( _page, _is_posted );

      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
			if( _is_posted ){
        __mqtt_service.handleMqttConfigChange(MQTT_PUBSUB_CONFIG);
      }
    }

};

#endif
