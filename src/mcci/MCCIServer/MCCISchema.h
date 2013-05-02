#pragma once

#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include "LinearHash.h"

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

    void load(sqlite3* schema_db);

    // get a hash that describes the working variable set
    string get_hash();

  protected:

    LinearHash<unsigned int, unsigned int> m_index_of_variable;  // convert variable_id to an index in our array

    sqlite3* m_db;


};
