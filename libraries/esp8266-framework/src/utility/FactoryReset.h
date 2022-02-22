/****************************** Reset Factory ********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _DEVICE_FACTORY_RESET_H_
#define _DEVICE_FACTORY_RESET_H_

#include <Esp.h>
#include <config/Config.h>
#include "Log.h"

struct factory_reset_cb_ {
	CallBackVoidArgFn _cb;
};


/**
 * DeviceFactoryReset class
 */
class DeviceFactoryReset{

	public:

		/**
		 * DeviceFactoryReset constructor
		 */
		DeviceFactoryReset();
		/**
		 * DeviceFactoryReset destructor
		 */
		~DeviceFactoryReset();

		void factory_reset( void );
		bool run_while_factory_reset( CallBackVoidArgFn _fn );
		void reset_device( int _timeout=0 );
		void restart_device( int _timeout=0 );
		void handleFlashKeyPress( void );

	protected:

		/**
		 * @var	uint8_t|0 m_flash_key_pressed
		 */
    uint8_t m_flash_key_pressed;
		/**
		 * @var	std::vector<factory_reset_cb_>	m_factory_reset_cbs
		 */
		std::vector<factory_reset_cb_> m_factory_reset_cbs;
};

extern DeviceFactoryReset __factory_reset;

#endif
