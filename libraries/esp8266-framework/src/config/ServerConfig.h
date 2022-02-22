/*************************** Server Config page *******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/
#ifndef _SERVER_CONFIG_H_
#define _SERVER_CONFIG_H_

#include "Common.h"

#define DEFAULT_LOGIN_USERNAME     USER
#define DEFAULT_LOGIN_PASSWORD     PASSPHRASE

#define EW_SESSION_NAME      "ew_session"
#define EW_COOKIE_MAX_AGE    300

#define LOGIN_CONFIGS_BUF_SIZE 25

struct login_credential {
  char username[LOGIN_CONFIGS_BUF_SIZE];
  char password[LOGIN_CONFIGS_BUF_SIZE];
  char session_name[LOGIN_CONFIGS_BUF_SIZE];
  uint16_t cookie_max_age;
};

const login_credential PROGMEM _login_credential_defaults = {
  DEFAULT_LOGIN_USERNAME,  DEFAULT_LOGIN_PASSWORD, EW_SESSION_NAME, EW_COOKIE_MAX_AGE
};

const int login_credential_size = sizeof(login_credential) + 5;

using login_credential_table = login_credential;

#endif
