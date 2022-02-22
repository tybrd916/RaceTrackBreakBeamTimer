/****************************** Database **************************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "Database.h"

int DatabaseTableAbstractLayer::m_total_instances = 0;
DatabaseTableAbstractLayer* DatabaseTableAbstractLayer::m_instances[MAX_TABLES] = {nullptr};

/**
 * initialize database with size as input parameter.
 *
 * @param uint16_t|DATABASE_MAX_SIZE  _size
 */
void Database::init_database(  uint16_t _size ){

  beginConfigs(_size);
  this->m_database_tables.reserve(MAX_TABLES);

  for (size_t i = 0; i < DatabaseTableAbstractLayer::m_total_instances; i++) {
    DatabaseTableAbstractLayer::m_instances[i]->boot();
  }

  #ifdef AUTO_FACTORY_RESET_ON_INVALID_CONFIGS

  __task_scheduler.setInterval( [&]() {

    if ( !isValidConfigs() ){

      #ifdef EW_SERIAL_LOG
      Log( F("\n\nFound invalid configs.. starting factory reset..!\n\n") );
      #endif
      __factory_reset.factory_reset();
    }

   }, MILLISECOND_DURATION_5000 );

  #endif
}

/**
 * clear all tables in database
 *
 * @param   uint16_t  _address
 * @return  bool
 */
void Database::clear_all(){

	for ( uint8_t i = 0; i < this->m_database_tables.size(); i++) {

		if( nullptr != this->m_database_tables[i].m_instance ){
      this->m_database_tables[i].m_instance->clear();
    }
	}
}

/**
 * register table in database tables by their address
 *
 * @param   struct_tables _table
 * @return  bool
 */
bool Database::register_table( struct_tables &_table ) {

  struct_tables _last = this->get_last_table();
  if(
    ( _last.m_table_address + _last.m_table_size + 2 ) < _table.m_table_address &&
    ( _table.m_table_address + _table.m_table_size + 2 ) < DATABASE_MAX_SIZE
  ){
    this->m_database_tables.push_back(_table);
    return true;
  }
  return false;
}

/**
 * get last registered table from database tables
 *
 * @return   struct_tables
 */
struct_tables Database::get_last_table(){

  struct_tables _last;
  uint8_t _last_add = 0;
  memset( &_last, 0, sizeof(struct_tables));
  for ( uint8_t i = 1; i < this->m_database_tables.size(); i++) {

    if( this->m_database_tables[_last_add].m_table_address < this->m_database_tables[i].m_table_address ){
      _last_add = i;
    }
  }
  if( _last_add != 0 ) return this->m_database_tables[_last_add]; return _last;
}

Database __database;
