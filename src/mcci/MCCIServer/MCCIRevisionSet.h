
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

    int getRevision(int variableID);  // return current value of revision
    int incRevision(int variableID);  // increment revision and return value

    // TODO: warn if the array will be shrunk
    void setMax(int variableMaxID) { m_cache.resize(variableMaxID, -1); };

    // the signature of the schema we're using
    string getSignature();
    void setSignature(string signature);

    bool getStrict() { return m_strict; };    // whether to allow signature changes
    void setStrict(bool v) { m_strict = v; };
    
  protected:    
    void checkRevision(int variableID); // put a variable in the DB if it's not there
        
};


#endif // !defined(MCCI_REVISION_SET_H_INCLUDED)
