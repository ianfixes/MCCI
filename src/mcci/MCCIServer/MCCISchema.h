#pragma once

#include <string>
#include <sqlite3.h>
#include "LinearHash.h"
#include "MCCITypes.h"
#include <vector>

using namespace std;

/**
   The Schema provides one of the core assumptions of MCCI message routing:
   assurance that all clients on all nodes have the same understanding of the
   meaning (well, the identification) of variables and the encoding of their
   values.
   
   This class provides the working set of variables and their types.
   It loads a schema from an sqlite3 database and keeps track of the
   ordinal numbers of each variable ID.  It does this because there may be gaps
   in the variable id, so this class can be thought of as a "packing" tool as
   well as a consistency tool between clients, etc.

   A hashing function is also provided, to ensure runtime compatibility of server and clients.
 */
class CMCCISchema
{

  protected:

    LinearHash<MCCI_VARIABLE_T, unsigned int> m_ordinality; // ordinality - variable ot ordinal
    vector<MCCI_VARIABLE_T>                   m_variable;   // ordinal to variable
    vector<string>                            m_name;       // the name of a variable, ordinal idx

    string m_hashval; // the hashed contents of the schema
    
  public:
    CMCCISchema(sqlite3* schema_db) { this->load(schema_db); }
    ~CMCCISchema() {}

    // populate the db
    void load(sqlite3* schema_db);

    unsigned int load_cardinality(sqlite3* schema_db);
    
    // get a hash that describes the working variable set
    string get_hash() { return m_hashval; }

    // the number of variables being used
    unsigned int get_cardinality() { return m_name.size(); }

    // the ordinality of a variable
    unsigned int ordinality_of_variable(MCCI_VARIABLE_T variable_id)
    {
        if (!m_ordinality.has_key(variable_id)) throw string("Tried to get ordinality of unknown var");
        return m_ordinality[variable_id];
    }

    // the variable of the ordinal
    MCCI_VARIABLE_T variable_of_ordinal(unsigned int ord) { return m_variable.at(ord); }
    
    // lookup the name of a variable
    string name_of_variable(MCCI_VARIABLE_T variable_id)
    { return m_name.at(ordinality_of_variable(variable_id)); }

                            
    static void b64_encode(unsigned char* in,
                           char* out,
                           unsigned int in_len,
                           unsigned int out_len);
    

};
