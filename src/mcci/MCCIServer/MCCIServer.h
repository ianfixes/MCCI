
#if !defined(MCCI_SERVER_H_INCLUDED)
#define MCCI_SERVER_H_INCLUDED

#include "FibbonacciHeap.cpp"
#include "MCCISchema.h"
#include <map>
#include <sqlite3.h>
//#include <unordered_map> // replace with boost?


typedef struct
{
    int  maxLocalRequests;
    int  maxRemoteRequests;

    CMCCISchema* schema;

} SMCCIServerSettings;


class CMCCIServer
{

public:
    CMCCIServer(SMCCIServerSettings settings);
    ~CMCCIServer();


protected:
    SMCCIServerSettings m_settings;

    // local requests: 
    // 
    //       array        myhash       list
    // reqs[variable_id][sequence_id][subscriber] = 



    // remote requests
    //
    //
    // reqs[host, variable_id][sequence_id][subscriber] = 

    
    // wildcard requests = subscriptions for duration of timeout
    //
    // host can be wildcard
    // variable can be wildcard
    // seq can be wildcard (i.e. latest)
    //
    // reqs


};


#endif // !defined(MCCI_SERVER_H_INCLUDED)
