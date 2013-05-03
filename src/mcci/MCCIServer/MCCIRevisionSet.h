#pragma once

#include <string>
#include <sqlite3.h>
#include "LinearHash.h"
#include "MCCITypes.h"

using namespace std;

/**
   A revision set is the keystone of sane server operation.  It ensures that sequence numbers
   increase appropriately (for uniqueness) including across database crashes or restarts.

   Note that the revision set must be aware of changes to the schema between runs!
 */
class CMCCIRevisionSet
{
  protected:

    sqlite3* m_db;
    sqlite3_stmt* m_insert;
    sqlite3_stmt* m_read;
    sqlite3_stmt* m_update;
    
    LinearHash<MCCI_VARIABLE_T, MCCI_REVISION_T> m_cache; // the max current sequence number "in the wild"
    
    bool m_strict; // whether to bail if schema signatures don't match (default yes)
    

  public:
    CMCCIRevisionSet(sqlite3* revision_db, unsigned int schema_cardinality, string schema_signature);
    ~CMCCIRevisionSet();

    void load(sqlite3* revision_db);

    MCCI_REVISION_T get_revision(MCCI_VARIABLE_T variable_id);  // return current value of revision
    MCCI_REVISION_T inc_revision(MCCI_VARIABLE_T variable_id);  // increment revision and return value

    // the signature of the schema we're using
    string get_signature();
    void set_signature(string signature);

    // if false, disregards mismatches in schema signatures.  this should only be used for debugging.
    bool get_strict() { return m_strict; };    
    void set_strict(bool v) { m_strict = v; };
    
  protected:
    // put a variable in the DB if it's not there already
    void check_revision(MCCI_VARIABLE_T variable_id); 
        
};
