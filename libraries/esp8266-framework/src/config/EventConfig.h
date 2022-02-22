/*************************** Event Config page ********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/
#ifndef _EVENT_CONFIG_H_
#define _EVENT_CONFIG_H_

#include "Common.h"

/**
 * max event listener callbacks
 */
#define MAX_EVENT_LISTENERS	MAX_SCHEDULABLE_TASKS

/**
* available event names
*/
typedef enum event_name{

  EVENT_WIFI_STA_CONNECTED = EVENT_STAMODE_CONNECTED,
  EVENT_WIFI_STA_DISCONNECTED,
  EVENT_WIFI_STA_AUTHMODE_CHANGE,
  EVENT_WIFI_STA_GOT_IP,
  EVENT_WIFI_STA_DHCP_TIMEOUT,
  EVENT_WIFI_AP_STACONNECTED,
  EVENT_WIFI_AP_STADISCONNECTED,
  EVENT_WIFI_AP_PROBEREQRECVED,
  EVENT_WIFI_OPMODE_CHANGED,
  EVENT_WIFI_AP_DISTRIBUTE_STA_IP,
  EVENT_WIFI_MAX,
} event_name_t;

/**
* event listener struct type for event
*/
struct event_listener_t {
  int _event;
  CallBackVoidPointerArgFn _event_handler;
};

#endif
