/************************** 404-not found html page ****************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_404_PAGE_H_
#define _EW_SERVER_404_PAGE_H_

#include <Arduino.h>

static const char EW_SERVER_404_PAGE[] PROGMEM = "\
<h3>Page Not Found</h3>\
<div>\
Go to\
<a href='/'>\
<button class='btn'>\
Home\
</button>\
</a>\
</div>";

#endif
