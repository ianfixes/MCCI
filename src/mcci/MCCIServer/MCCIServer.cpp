
#include "MCCIServer.h"
#include <limits.h>

using namespace std;

// FIXME: replace 11111 with UINT32_MAX

CMCCIServer::CMCCIServer(SMCCIServerSettings settings)
{
    m_settings = settings;
}

bool CMCCIServer::is_rejectable_request(const SMCCIRequestPacket* input)
{
    return 0 < input->revision && (
        (0 == input->node_address && 0 == input->variable_id)  // requests with a sequence number but no variable
        || (11111 == input -> node_address && 0 < input->variable_id)); // requests with no host but yes variable
}


int CMCCIServer::process_request(int requestor_id, const SMCCIRequestPacket* input, SMCCIResponsePacket* response)
{
    response->requests_remaining_local  = client_free_requests_local(requestor_id);
    response->requests_remaining_remote = client_free_requests_remote(requestor_id);
    
    // process rejections first -- even before timeouts -- so that bad packets always reject
    if (is_rejectable_request(input))
    {
        response->accepted = false;
        return 1;
    }
    response->accepted = true;     // all requests are (or should be) OK after this
    
    // check the time 
    if (get_mcci_time() > input->timeout)
    {
        return 1;
    }
    
    // if subscribing to ALL nodes
    if (11111 == input->node_address) 
    {
        if (!response->requests_remaining_remote) return 1; // basically just drop silently
        
        if (0 == input->variable_id)
        {
            // promiscuous: subscribe to EVERYTHING
            subscribe_promiscuous(requestor_id, input->timeout); 
        }
        else
        {
            // "1 variable on all nodes" (discovery)
            subscribe_to_variable(requestor_id, input->timeout, input->variable_id); 
        }

        ++m_outstanding_requests_remote[requestor_id];
        response->requests_remaining_remote = client_free_requests_remote(requestor_id);
        return 1;
    }
    
    // subscribing to one node that may be local or remote
    if (0 == input->variable_id && 0 == input->revision)
    {
        if (!response->requests_remaining_remote) return 1; // silently drop request
        
        subscribe_to_host(requestor_id, input->timeout, input->node_address);
        response->requests_remaining_remote -= 1;
        return 1;
    }
    
    return process_forwardable_request(requestor_id, input, response);
}


int CMCCIServer::process_forwardable_request(int requestor_id, const SMCCIRequestPacket* input, SMCCIResponsePacket* response)
{
    //TODO: assert input->variable_id != 0
    //TODO: assert node_address != ALL
    // response->requests_remaining has already been initialized
    
    bool is_for_me = is_my_address(input->node_address);
    bool do_forward = !is_for_me;
    
    if (!is_for_me && 0 == input->revision)
    {
        if (!response->requests_remaining_remote) return 1; //silently drop
        
        subscribe_to_host_var(requestor_id, input->timeout, input->node_address, input->variable_id);
        forward_request(requestor_id, input);
    }           
    // guaranteed to have a measurable value for revision=0 at this point
    
    int firstrev;
    int direction = (input->quantity > 0) - (input->quantity < 0);  // extracts sign
    int quantity  = input->quantity * direction;  // cancels any negative sign
    int limit;

    // calculate limit -- max quantity allowed.  basically same calculation using 
    if (is_for_me)
        limit = response->requests_remaining_local >= quantity ? quantity : response->requests_remaining_local;
    else
        limit = response->requests_remaining_local >= quantity ? quantity : response->requests_remaining_remote;

    // figure out where to start counting (observing desired range and order, constrained by limits)
    if (0 < input->revision)
        if (1 == direction)
            firstrev = input->revision;
        else
            firstrev = input->revision + quantity - limit;
    else
    {
        int maxrevision = m_settings.revisionset->get_revision(input->variable_id);
        if (1 == direction)
            firstrev = maxrevision - quantity + 1;
        else
            firstrev = maxrevision - limit + 1;
    }
    int lastrev = firstrev + limit - 1;

    
    // expand subscription range and add to various queues
    for (int r = firstrev; r <= lastrev; r++)
    {
        if (is_for_me)
        {
            do_forward |= !is_in_working_set(input->variable_id); // if we don't have this value, ask for it
            subscribe_specific(requestor_id, input->timeout, input->variable_id, r);
        }
        else
        {
            subscribe_specific_remote(requestor_id, input->timeout, input->node_address, input->variable_id, r);
        }
    }


    // decrement the remaining requests
    // FIXME, this should mess with the vector
    int rem;
    if (is_for_me)
    {
        rem = response->requests_remaining_local;
        response->requests_remaining_local = quantity <= rem ? rem - quantity : 0;
    }
    else
    {
        rem = response->requests_remaining_remote;
        response->requests_remaining_remote = quantity <= rem ? rem - quantity : 0;
    }
    
    if (do_forward) forward_request(requestor_id, input);

    return 1;
}


int CMCCIServer::process_data(int provider_id, const SMCCIDataPacket* input)
{
    return 1;   
}

int CMCCIServer::process_production(int provider_id, const SMCCIProductionPacket* input, SMCCIAcceptancePacket* output)
{
    return 1;
}

    
