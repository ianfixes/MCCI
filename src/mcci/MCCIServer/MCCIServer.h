#pragma once

#include "FibbonacciHeap.cpp"
#include "MCCISchema.h"
#include "MCCIRevisionSet.h"
#include "MCCITime.h"
#include "packets.h"
#include <map>
#include <vector>
#include <sqlite3.h>
//#include <unordered_map> // replace with boost?


typedef struct
{
    int my_node_address;
    int max_local_requests;
    int max_remote_requests;

    CMCCISchema* schema;
    CMCCIRevisionSet* revisionset;

} SMCCIServerSettings;


class CMCCIServer
{
    
  protected:
    SMCCIServerSettings m_settings;
    
    vector<int> m_outstanding_requests_local;   // requests per subscriber
    vector<int> m_outstanding_requests_remote;  // requests per subscriber

    vector<SMCCIDataPacket*> m_working_set; // current values of stuff
    
  public:
    CMCCIServer(SMCCIServerSettings settings);
    ~CMCCIServer();
    
    int process_request(int requestor_id, const SMCCIRequestPacket* input, SMCCIResponsePacket* response);
    int process_data(int provider_id, const SMCCIDataPacket* input);
    int process_production(int provider_id, const SMCCIProductionPacket* input, SMCCIAcceptancePacket* output);
    
    int client_free_requests_local(int client_id)  { return m_settings.max_local_requests - m_outstanding_requests_local[client_id]; }
    int client_free_requests_remote(int client_id) { return m_settings.max_remote_requests - m_outstanding_requests_remote[client_id]; }
    
    
    
  protected:
    
    bool is_my_address(int address) { return 0 == address || address == m_settings.my_node_address; };
    bool is_rejectable_request(const SMCCIRequestPacket* input);
    bool is_in_working_set(int variable_id) { return NULL != m_working_set[variable_id]; };
    
    int process_forwardable_request(int requestor_id, const SMCCIRequestPacket* input, SMCCIResponsePacket* response);
    int forward_request(int requestor_id, const SMCCIRequestPacket* request) { return 1; };
    int subscribe_promiscuous(int client_id, MCCI_TIME_T timeout) { return 1; };
    int subscribe_to_variable(int client_id, MCCI_TIME_T timeout,
                              int variable_id) { return 1; };
    int subscribe_to_host(int client_id, MCCI_TIME_T timeout,
                          int node_address) { return 1; };
    int subscribe_to_host_var(int client_id, MCCI_TIME_T timeout,
                              int node_address, int variable_id) { return 1; };
    int subscribe_specific(int client_id, MCCI_TIME_T timeout, int variable_id, int revision) { return 1; };
    int subscribe_specific_remote(int client_id, MCCI_TIME_T timeout,
                                  int node_address, int variable_id, int revision) { return 1; };
    
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


