#pragma once

#include "FibonacciHeap.h"
#include "MCCISchema.h"
#include "MCCIRevisionSet.h"
#include "MCCITime.h"
#include "packets.h"
#include <map>
#include <vector>
#include <sqlite3.h>
//#include <unordered_map> // replace with boost?


// the settings for operating a MCCI server
typedef struct
{
    int my_node_address;
    int max_local_requests;
    int max_remote_requests;

    CMCCISchema* schema;
    CMCCIRevisionSet* revisionset;

} SMCCIServerSettings;


/**
   This class is the logical component of the MCCI system's packet request & delivery system.
 */
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
    
    // accept a request packet, and put its contents in the appropriate structures, responding accordingly
    int process_request(int requestor_id, const SMCCIRequestPacket* input, SMCCIResponsePacket* response);

    // accept a data packet, giving a reference to its contents to all necessary subscriber
    int process_data(int provider_id, const SMCCIDataPacket* input);

    // accept a production packet
    int process_production(int provider_id, const SMCCIProductionPacket* input, SMCCIAcceptancePacket* output);

    // tell the client how many requests it is allowed to make
    int client_free_requests_local(int client_id)  { return m_settings.max_local_requests - m_outstanding_requests_local[client_id]; }
    int client_free_requests_remote(int client_id) { return m_settings.max_remote_requests - m_outstanding_requests_remote[client_id]; }
    
    
    
  protected:

    // whether an address is equivalent to "localhost"
    bool is_my_address(int address) { return 0 == address || address == m_settings.my_node_address; };
    
    // whether a variable id has delivered its first value
    bool is_in_working_set(int variable_id) { return NULL != m_working_set[variable_id]; };

    // whether a request has one of the 4 possible input combinations that makes it wrong
    bool is_rejectable_request(const SMCCIRequestPacket* input);

    // slave to process_request, for the cases that involve forwarding
    int process_forwardable_request(int requestor_id, const SMCCIRequestPacket* input, SMCCIResponsePacket* response);

    // send a request to be delivered to all clients
    int forward_request(int requestor_id, const SMCCIRequestPacket* request) { return 1; };

    // add a client to the list of recipients for all data packets
    int subscribe_promiscuous(int client_id, MCCI_TIME_T timeout) { return 1; }; // FIXME

    // add a client to the list of recipients for packets of given variable_id
    int subscribe_to_variable(int client_id, MCCI_TIME_T timeout,
                              int variable_id) { return 1; }; // FIXME

    // add a client to the list of recipients for packets from given host
    int subscribe_to_host(int client_id, MCCI_TIME_T timeout,
                          int node_address) { return 1; }; // FIXME

    // add a client to the list of recipients for packets from given host and variable ID
    int subscribe_to_host_var(int client_id, MCCI_TIME_T timeout,
                              int node_address, int variable_id) { return 1; }; // FIXME

    //add a client to the list of receipents for a specific packet from this host
    int subscribe_specific(int client_id, MCCI_TIME_T timeout, int variable_id, int revision) { return 1; }; // FIXME

    // add a client to the list of recipients for a specific packet from a remote host
    int subscribe_specific_remote(int client_id, MCCI_TIME_T timeout,
                                  int node_address, int variable_id, int revision) { return 1; }; // FIXME
    
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


