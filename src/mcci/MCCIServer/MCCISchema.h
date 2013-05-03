#pragma once

#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include "LinearHash.h"
#include "MCCITypes.h"

using namespace std;

/**
   This class provides the working set of variables and their types.  It loads a schema from an sqlite3 database
   and keeps track of the ordinal numbers of each variable ID.

   A hashing function is also provided, to ensure runtime compatibility of server and clients.
 */
class CMCCISchema
{

  public:
    CMCCISchema(sqlite3* schema_db) { this->load(schema_db); };
    ~CMCCISchema();

    // populate the db
    void load(sqlite3* schema_db);

    // get a hash that describes the working variable set
    string get_hash() { return m_hashval; };

    // the number of variables being used
    unsigned int get_cardinality() { return m_name.count(); }; 

    // lookup the name of a variable
    string name_of_variable(MCCI_VARIABLE_T variable_id) { return m_name[variable_id]; };

                            
  protected:

    LinearHash<MCCI_VARIABLE_T, string> m_name;  // the name of a variable

    string m_hashval;
    
    sqlite3* m_db;


};
