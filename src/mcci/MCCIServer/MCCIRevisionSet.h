
#if !defined(MCCI_REVISION_SET_H_INCLUDED)
#define MCCI_REVISION_SET_H_INCLUDED

#include <string>
#include <sqlite3.h>
#include <vector>

using namespace std;

class CMCCIRevisionSet
{
  protected:

    sqlite3* m_db;
    sqlite3_stmt* m_insert;
    sqlite3_stmt* m_read;
    sqlite3_stmt* m_update;
    
    vector<int> m_cache;
    
    bool m_strict; 
    

  public:
    CMCCIRevisionSet(sqlite3* revision_db, string schema_signature);
    ~CMCCIRevisionSet();

    void load(sqlite3* revision_db);

    int get_revision(int variable_id);  // return current value of revision
    int inc_revision(int variable_id);  // increment revision and return value

    // TODO: warn if the array will be shrunk
    void set_max(int variable_max_id) { m_cache.resize(variable_max_id, -1); };

    // the signature of the schema we're using
    string get_signature();
    void set_signature(string signature);

    bool get_strict() { return m_strict; };    // whether to allow signature changes
    void set_strict(bool v) { m_strict = v; };
    
  protected:    
    void check_revision(int variable_id); // put a variable in the DB if it's not there
        
};


#endif // !defined(MCCI_REVISION_SET_H_INCLUDED)
