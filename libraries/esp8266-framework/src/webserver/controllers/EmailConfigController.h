/************************** Email Config Controller ****************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_EMAIL_CONFIG_CONTROLLER_
#define _EW_SERVER_EMAIL_CONFIG_CONTROLLER_

#include "Controller.h"
#include <service_provider/EmailServiceProvider.h>
#include <webserver/pages/EmailConfigPage.h>
#include <webserver/pages/WiFiConfigPage.h>

#define	TEST_EMAIL_MESSAGE	"this is test mail from esp"

/**
 * EmailConfigController class
 */
class EmailConfigController : public Controller {

	protected:
		/**
		 * @var	email_config_table  email_configs
		 */
		email_config_table email_configs;


	public:
		/**
		 * EmailConfigController constructor
		 */
		EmailConfigController():Controller("email"){
		}

		/**
		 * EmailConfigController destructor
		 */
		~EmailConfigController(){
		}

		/**
		 * register email config controller
		 *
		 */
		void boot( void ){

			if( nullptr != this->m_web_resource && nullptr != this->m_web_resource->m_db_conn ){
				this->email_configs = this->m_web_resource->m_db_conn->get_email_config_table();
			}
			if( nullptr != this->m_route_handler ){
				this->m_route_handler->register_route( EW_SERVER_EMAIL_CONFIG_ROUTE, [&]() { this->handleEmailConfigRoute(); }, AUTH_MIDDLEWARE );
			}
		}

		/**
		 * build email config html.
		 *
		 * @param	char*	_page
		 * @param	bool|false	_is_error
		 * @param	bool|false	_enable_flash
		 * @param	bool|false	_is_test_mail
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
		void build_email_config_html( char* _page, bool _is_error=false, bool _enable_flash=false, bool _is_test_mail=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_EMAIL_CONFIG_PAGE_TOP );

			char _port[10];memset( _port, 0, 10 );
			__appendUintToBuff( _port, "%d", this->email_configs.mail_port, 8 );
			// char _freq[10];memset( _freq, 0, 10 );
      // __appendUintToBuff( _freq, "%d", this->email_configs.mail_frequency, 8 );

      concat_tr_input_html_tags( _page, PSTR("Domain:"), PSTR("ml_dmn"), this->email_configs.sending_domain, DEFAULT_SENDING_DOMAIN_MAX_SIZE-1 );
      concat_tr_input_html_tags( _page, PSTR("Mail Server:"), PSTR("ml_srvr"), this->email_configs.mail_host, DEFAULT_MAIL_HOST_MAX_SIZE-1 );

      concat_tr_input_html_tags( _page, PSTR("Mail Port:"), PSTR("ml_prt"), _port );
      concat_tr_input_html_tags( _page, PSTR("Mail Username:"), PSTR("ml_usr"), this->email_configs.mail_username, DEFAULT_MAIL_USERNAME_MAX_SIZE-1 );
      concat_tr_input_html_tags( _page, PSTR("Mail Password:"), PSTR("ml_psw"), this->email_configs.mail_password, DEFAULT_MAIL_PASSWORD_MAX_SIZE-1, (char*)"password" );

      concat_tr_input_html_tags( _page, PSTR("Mail From:"), PSTR("ml_frm"), this->email_configs.mail_from, DEFAULT_MAIL_FROM_MAX_SIZE-1 );
      concat_tr_input_html_tags( _page, PSTR("Mail From Name:"), PSTR("ml_frnm"), this->email_configs.mail_from_name, DEFAULT_MAIL_FROM_NAME_MAX_SIZE-1 );

      concat_tr_input_html_tags( _page, PSTR("Mail To:"), PSTR("ml_to"), this->email_configs.mail_to, DEFAULT_MAIL_TO_MAX_SIZE-1 );
      concat_tr_input_html_tags( _page, PSTR("Mail Subject:"), PSTR("ml_sub"), this->email_configs.mail_subject, DEFAULT_MAIL_SUBJECT_MAX_SIZE-1 );
			concat_tr_input_html_tags( _page, PSTR("Send Test Mail ?"), PSTR("tstml"), "test", HTML_INPUT_TAG_DEFAULT_MAXLENGTH, HTML_INPUT_CHECKBOX_TAG_TYPE, false );
      // concat_tr_input_html_tags( _page, PSTR("Mail Frequency:"), PSTR("ml_freq"), _freq );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );

      if( _enable_flash )
      concat_flash_message_div( _page, _is_error ? PSTR("Invalid length error"): _is_test_mail ? HTML_EMAIL_SUCCESS_FLASH : HTML_SUCCESS_FLASH, _is_error ? ALERT_DANGER:ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
    }

		/**
		 * build and send email config page.
		 * when posted, get email configs from client and set them in database.
		 */
    void handleEmailConfigRoute( void ) {

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ||
					nullptr == this->m_web_resource->m_server ||
					nullptr == this->m_route_handler ){
				return;
			}

			this->email_configs = this->m_web_resource->m_db_conn->get_email_config_table();
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Email Config route"));
      #endif
      bool _is_posted = false;
			bool _is_test_mail = false;
      bool _is_error = true;

      if ( this->m_web_resource->m_server->hasArg("ml_dmn") && this->m_web_resource->m_server->hasArg("ml_srvr") ) {

        String _mail_domain = this->m_web_resource->m_server->arg("ml_dmn");
				String _mail_server = this->m_web_resource->m_server->arg("ml_srvr");
				String _mail_port = this->m_web_resource->m_server->arg("ml_prt");
				String _mail_username = this->m_web_resource->m_server->arg("ml_usr");
				String _mail_password = this->m_web_resource->m_server->arg("ml_psw");
				String _mail_from = this->m_web_resource->m_server->arg("ml_frm");
				String _mail_from_name = this->m_web_resource->m_server->arg("ml_frnm");
				String _mail_to = this->m_web_resource->m_server->arg("ml_to");
				String _mail_subject = this->m_web_resource->m_server->arg("ml_sub");
				// String _mail_frequency = this->m_web_resource->m_server->arg("ml_freq");
				String _test_mail = this->m_web_resource->m_server->arg("tstml");

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
          Log(F("mail domain : ")); Logln( _mail_domain );
          Log(F("mail server : ")); Logln( _mail_server );
          Log(F("mail port : ")); Logln( _mail_port );
          Log(F("mail username : ")); Logln( _mail_username );
          Log(F("mail password : ")); Logln( _mail_password );
          Log(F("mail from : ")); Logln( _mail_from );
          Log(F("mail from name : ")); Logln( _mail_from_name );
          Log(F("mail to : ")); Logln( _mail_to );
          Log(F("mail subject : ")); Logln( _mail_subject );
          // Log(F("mail frequency : ")); Logln( _mail_frequency );
					Log(F("test mail : ")); Logln( _test_mail );
          Logln();
        #endif

				if( _mail_domain.length() <= DEFAULT_SENDING_DOMAIN_MAX_SIZE && _mail_server.length() <= DEFAULT_MAIL_HOST_MAX_SIZE &&
          _mail_username.length() <= DEFAULT_MAIL_USERNAME_MAX_SIZE && _mail_password.length() <= DEFAULT_MAIL_PASSWORD_MAX_SIZE &&
          _mail_from.length() <= DEFAULT_MAIL_FROM_MAX_SIZE && _mail_from_name.length() <= DEFAULT_MAIL_FROM_NAME_MAX_SIZE &&
          _mail_to.length() <= DEFAULT_MAIL_TO_MAX_SIZE && _mail_subject.length() <= DEFAULT_MAIL_SUBJECT_MAX_SIZE
        ){

          _ClearObject( &this->email_configs );

          _mail_domain.toCharArray( this->email_configs.sending_domain, _mail_domain.length()+1 );
          _mail_server.toCharArray( this->email_configs.mail_host, _mail_server.length()+1 );
					this->email_configs.mail_port = (uint16_t)_mail_port.toInt();
					_mail_username.toCharArray( this->email_configs.mail_username, _mail_username.length()+1 );
					_mail_password.toCharArray( this->email_configs.mail_password, _mail_password.length()+1 );
					_mail_from.toCharArray( this->email_configs.mail_from, _mail_from.length()+1 );
					_mail_from_name.toCharArray( this->email_configs.mail_from_name, _mail_from_name.length()+1 );
					_mail_to.toCharArray( this->email_configs.mail_to, _mail_to.length()+1 );
					_mail_subject.toCharArray( this->email_configs.mail_subject, _mail_subject.length()+1 );
					// this->email_configs.mail_frequency = (uint16_t)_mail_frequency.toInt();
					_is_test_mail = (bool)( _test_mail == "test" );

          this->m_web_resource->m_db_conn->set_email_config_table( &this->email_configs );
          _is_error = false;
        }
        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_email_config_html( _page, _is_error, _is_posted, _is_test_mail );

      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;

			if( _is_posted && !_is_error && _is_test_mail ){
				__email_service.sendMail( TEST_EMAIL_MESSAGE );
				// __email_service.sendMail( PSTR("this is test mail") );
      }
    }

};

#endif
