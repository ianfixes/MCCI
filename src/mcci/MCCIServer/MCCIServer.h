
#if !defined(MCCI_SERVER_H_INCLUDED)
#define MCCI_SERVER_H_INCLUDED

#include "FibbonacciHeap.cpp"
#include "MCCISchema.h"
#include "MCCIRevisionSet.h"
#include "packets.h"
#include <map>
#include <vector>
#include <sqlite3.h>
//#include <unordered_map> // replace with boost?


typedef struct
{
    int myNodeAddress;
    int maxLocalRequests;
    int maxRemoteRequests;

    CMCCISchema* schema;
    CMCCIRevisionSet* revisionset;

} SMCCIServerSettings;


class CMCCIServer
{
    
  protected:
    SMCCIServerSettings m_settings;
    
    vector<int> m_outstandingRequestsLocal;   // requests per subscriber
    vector<int> m_outstandingRequestsRemote;  // requests per subscriber
    
  public:
    CMCCIServer(SMCCIServerSettings settings);
    ~CMCCIServer();
    
    int processRequest(int requestorID, const SMCCIRequestPacket* input, SMCCIResponsePacket* response);
    int processData(int providerID, const SMCCIDataPacket* input);
    int processProduction(int providerID, const SMCCIProductionPacket* input, SMCCIAcceptancePacket* output);
    
    int clientFreeRequestsLocal(int clientID)  { return m_settings.maxLocalRequests - m_outstandingRequestsLocal[clientID]; }
    int clientFreeRequestsRemote(int clientID) { return m_settings.maxRemoteRequests - m_outstandingRequestsRemote[clientID]; }
    
    
    
  protected:
    
    bool isMyAddress(int address) { return 0 == address || address == m_settings.myNodeAddress; };

    int forwardRequest(const SMCCIRequestPacket* request) { return 1; };
    int subscribePromiscuous(int clientID) { return 1; };
    int subscribeToVariable(int clientID, int variableID) { return 1; };
    int subscribeToHost(int clientID, int nodeAddress) { return 1; };
    int subscribeToHostVar(int clientID, int nodeAddress, int variableID) { return 1; };
    int subscribeSpecific(int clientID, int nodeAddress, int variableID, int revision) { return 1; };
    
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
