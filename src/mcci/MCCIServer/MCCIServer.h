
#if !defined(MCCI_SERVER_H_INCLUDED)
#define MCCI_SERVER_H_INCLUDED

#include "FibbonacciHeap.cpp"
#include <map>
#include <sqlite3.h>
//#include <unordered_map> // replace with boost?


typedef struct
{
    bool hashRemoteRequests;
    int  maxLocalRequests;
    int  maxRemoteRequests;
    
    sqlite3* schema;

} SMCCIServerSettings;


class CMCCIServer
{

public:
    CMCCIServer(SMCCIServerSettings settings);
    ~CMCCIServer();


protected:
    SMCCIServerSettings m_settings;
    int* m_indexOfVariable;  // convert variable_id to an index in our array


    void buildSchema();

};


#endif // !defined(MCCI_SERVER_H_INCLUDED)
