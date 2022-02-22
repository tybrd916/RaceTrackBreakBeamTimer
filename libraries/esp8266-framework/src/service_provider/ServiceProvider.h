/***************************** service provider *******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _SERVICE_PROVIDER_H_
#define _SERVICE_PROVIDER_H_


#include <interface/WiFiInterface.h>
#include <interface/WiFiClientInterface.h>
#include <interface/HttpClientInterface.h>
#include <interface/iDeviceIotInterface.h>

#include <config/Config.h>
#include <utility/Utility.h>
#include <database/DefaultDatabase.h>
extern "C" {
#include "user_interface.h"
}

/**
 * ServiceProvider class
 */
class ServiceProvider{

  public:

    /**
     * ServiceProvider constructor.
     */
    ServiceProvider(){
    }

    /**
		 * ServiceProvider destructor
		 */
    ~ServiceProvider(){
    }
};

#endif
