#pragma once

#include <string>
#include <sqlite3.h>
#include "LinearHash.h"
#include "MCCITypes.h"

using namespace std;

/**
   The RevisionSet provides one of the core assumptions of MCCI message routing:
   assurance that revisions of variables created on a node do not produce duplicates.
   
   In other words, it ensures that sequence numbers
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

    long m_signature_id; // signature ID to use

  public:
    CMCCIRevisionSet(sqlite3* revision_db, unsigned int schema_cardinality, string schema_signature);
    CMCCIRevisionSet(const CMCCIRevisionSet &rhs);
    ~CMCCIRevisionSet();

    // output operator
    friend ostream& operator<<(ostream &out, CMCCIRevisionSet const &rhs);
    
    void load(sqlite3* revision_db);

    // return current value of revision
    MCCI_REVISION_T get_revision(MCCI_VARIABLE_T variable_id);
    
    // increment revision and return value
    MCCI_REVISION_T inc_revision(MCCI_VARIABLE_T variable_id);  

    // the signature of the schema we're using
    string get_signature() const;

    // put a signature in the DB if it's not there already and retain its id
    void set_signature(string signature);

    // if false, disregards mismatches in schema signatures.
    //      this should ONLY be used for debugging.
    bool get_strict() const { return m_strict; };    
    void set_strict(bool v) { m_strict = v; };
    
  protected:
    // put a variable in the DB if it's not there already
    void check_revision(MCCI_VARIABLE_T variable_id);

    // DB query for signature id, returns 0 if none exists
    int lookup_signature_id(string signature);
    
};
