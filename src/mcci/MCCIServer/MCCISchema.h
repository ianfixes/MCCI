
#if !defined(MCCI_SCHEMA_H_INCLUDED)
#define MCCI_SCHEMA_H_INCLUDED

#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
//#include <unordered_map> // replace with boost?
#include <vector>

using namespace std;

class CMCCISchema
{

  public:
    CMCCISchema(sqlite3* schema_db) { this->load(schema_db); };
    ~CMCCISchema();

    void load(sqlite3* schema_db);

    // get a hash that describes the working variable set
    string get_hash();

  protected:
    
    vector<int> m_index_of_variable;  // convert variable_id to an index in our array

    sqlite3* m_db;


};


#endif // !defined(MCCI_SCHEMA_H_INCLUDED)
