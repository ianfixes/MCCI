
#if !defined(MCCI_SCHEMA_H_INCLUDED)
#define MCCI_SCHEMA_H_INCLUDED

#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
//#include <unordered_map> // replace with boost?

using namespace std;

class CMCCISchema
{

public:
    CMCCISchema();
    ~CMCCISchema();

    // get access to a database
    void load(sqlite3* schema_db) { m_db = schema_db; };
    bool isLoaded() { return m_db != NULL; };

    // get a hash that describes the working variable set
    string getHash();

protected:

    int* m_indexOfVariable;  // convert variable_id to an index in our array

    sqlite3* m_db;


};


#endif // !defined(MCCI_SCHEMA_H_INCLUDED)
