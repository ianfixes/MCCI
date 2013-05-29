
#include "MCCIServer.h"


using namespace std;



CMCCIServer::CMCCIServer(SMCCIServerSettings settings) :
    m_bank_all(100, 1),
    m_bank_host(settings.max_clients, settings.bank_size_host),
    m_bank_var(settings.max_clients, settings.bank_size_var),
    m_bank_hostvar(settings.max_clients, settings.bank_size_hostvar),
    m_bank_remote(settings.max_clients, settings.bank_size_remote_hostvar, settings.bank_size_remote_rev),
    m_bank_varrev(settings.max_clients, settings.bank_size_varrev_var, settings.bank_size_varrev_rev)

{
    m_settings = settings;
}

bool CMCCIServer::is_rejectable_request(const SMCCIRequestPacket* input)
{
    return 0 < input->revision && (
        // requests with a sequence number but no variable
        (0 == input->node_address && 0 == input->variable_id)
        // requests with no host but yes variable
        || (MCCI_HOST_ANY == input -> node_address && 0 < input->variable_id)); 
}


int CMCCIServer::process_request(MCCI_CLIENT_ID_T requestor_id,
                                 const SMCCIRequestPacket* input,
                                 SMCCIResponsePacket* response)
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
    if (MCCI_HOST_ANY == input->node_address) 
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


int CMCCIServer::process_forwardable_request(MCCI_CLIENT_ID_T requestor_id,
                                             const SMCCIRequestPacket* input,
                                             SMCCIResponsePacket* response)
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
    
    MCCI_REVISION_T firstrev;
    int direction = (input->quantity > 0) - (input->quantity < 0);  // extracts sign
    unsigned int quantity  = input->quantity * direction;  // cancels any negative sign
    unsigned int limit;

    // calculate limit -- max quantity allowed.  basically same calculation using local vs remote
    if (is_for_me)
        limit = response->requests_remaining_local >= quantity ?
            quantity : response->requests_remaining_local;
    else
        limit = response->requests_remaining_local >= quantity ?
            quantity : response->requests_remaining_remote;

    // figure out where to start counting (observing desired range and order, constrained by limits)
    if (0 < input->revision)
        if (1 == direction)
            firstrev = input->revision;
        else
            firstrev = input->revision + quantity - limit;
    else
    {
        MCCI_REVISION_T maxrevision = m_settings.revisionset->get_revision(input->variable_id);
        if (1 == direction)
            firstrev = maxrevision - quantity + 1;
        else
            firstrev = maxrevision - limit + 1;
    }
    MCCI_REVISION_T lastrev = firstrev + limit - 1;

    
    // expand subscription range and add to various queues
    for (MCCI_REVISION_T r = firstrev; r <= lastrev; r++)
    {
        if (is_for_me)
        {
            // if we don't have this value, ask for it
            do_forward |= !is_in_working_set(input->variable_id); 
            subscribe_specific(requestor_id, input->timeout, input->variable_id, r);
        }
        else
        {
            subscribe_specific_remote(requestor_id,
                                      input->timeout,
                                      input->node_address,
                                      input->variable_id, r);
        }
    }


    // decrement the remaining requests
    unsigned int rem;
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


int CMCCIServer::subscribe_promiscuous(MCCI_CLIENT_ID_T client_id, MCCI_TIME_T timeout)
{
    m_bank_all.add(1, client_id, timeout);
    return 1;
}

int CMCCIServer::subscribe_to_host(MCCI_CLIENT_ID_T client_id,
                                   MCCI_TIME_T timeout,
                                   MCCI_NODE_ADDRESS_T node_address)
{
    m_bank_host.add(node_address, client_id, timeout);
    return 1;
}

int CMCCIServer::subscribe_to_variable(MCCI_CLIENT_ID_T client_id,
                                       MCCI_TIME_T timeout,
                                       MCCI_VARIABLE_T variable_id)
{
    m_bank_var.add(variable_id, client_id, timeout);
    return 1;
}

int CMCCIServer::subscribe_to_host_var(MCCI_CLIENT_ID_T client_id,
                                       MCCI_TIME_T timeout,
                                       MCCI_NODE_ADDRESS_T host,
                                       MCCI_VARIABLE_T variable_id)
{
    HostVarPair hv;
    hv.host = host;
    hv.var = variable_id;
    m_bank_hostvar.add(hv, client_id, timeout);

    return 1;
}

int CMCCIServer::subscribe_specific_remote(MCCI_CLIENT_ID_T client_id,
                                           MCCI_TIME_T timeout,
                                           MCCI_NODE_ADDRESS_T host,
                                           MCCI_VARIABLE_T variable_id,
                                           MCCI_REVISION_T revision)
{
    HostVarRevTuple hvr;
    hvr.host = host;
    hvr.var = variable_id;
    hvr.rev = revision;
    m_bank_remote.add(hvr, client_id, timeout);

    return 1;
}


int CMCCIServer::subscribe_specific(MCCI_CLIENT_ID_T client_id,
                                    MCCI_TIME_T timeout,
                                    MCCI_VARIABLE_T variable_id,
                                    MCCI_REVISION_T revision)
{
    VarRevPair vr;
    vr.var = variable_id;
    vr.rev = revision;
    m_bank_varrev.add(vr, client_id, timeout);
    return 1;
}


int CMCCIServer::process_production(MCCI_CLIENT_ID_T provider_id,
                                    const SMCCIProductionPacket* input,
                                    SMCCIAcceptancePacket* output)
{
    // hit the revisionset for the revision id
    // add the packet to the working set
    // call process_data with the new packet
    return 1;
}


int CMCCIServer::process_data(MCCI_CLIENT_ID_T provider_id, const SMCCIDataPacket* input)
{

    // create linear hash
    LinearHash<int, bool> hits(100);

    // check all request banks for client matches
    for (AllRequestBank::subscriber_iterator it = m_bank_all.subscribers_begin(1);
         it != m_bank_all.subscribers_end(1); ++it)
    {
        hits[*it] = true;
    }

    for (HostRequestBank::subscriber_iterator it = m_bank_host.subscribers_begin(input->node_address);
         it != m_bank_host.subscribers_end(input->node_address); ++it)
    {
        hits[*it] = true;
    }

    for (VariableRequestBank::subscriber_iterator it = m_bank_var.subscribers_begin(input->variable_id);
         it != m_bank_var.subscribers_end(input->variable_id); ++it)
    {
        hits[*it] = true;
    }

    HostVarPair hv;
    hv.host = input->node_address;
    hv.var  = input->variable_id;
    for (HostVariableRequestBank::subscriber_iterator it = m_bank_hostvar.subscribers_begin(hv);
         it != m_bank_hostvar.subscribers_end(hv); ++it)
    {
        hits[*it] = true;
    }

    VarRevPair vr;
    vr.var = input->variable_id;
    vr.rev = input->revision;
    for (VariableRevisionRequestBank::subscriber_iterator it = m_bank_varrev.subscribers_begin(vr);
         it != m_bank_varrev.subscribers_end(vr); ++it)
    {
        hits[*it] = true;
    }

    
    // iterate over linear hash and send data to clients

    // process deletions from banks
    
    return 1;
}


unsigned int CMCCIServer::client_free_requests_local(unsigned short client_id)
{
    return m_settings.max_local_requests - (
        0 // m_bank_all
        + m_bank_varrev.get_outstanding_request_count(client_id)
        );  // FIXME
}
    
unsigned int CMCCIServer::client_free_requests_remote(unsigned short client_id)
{
    return m_settings.max_remote_requests - (
        0 // m_bank_all
        + m_bank_host.get_outstanding_request_count(client_id)
        + m_bank_var.get_outstanding_request_count(client_id)
        + m_bank_hostvar.get_outstanding_request_count(client_id)
        );  // FIXME
}

int CMCCIServer::enforce_timeouts()
{
    MCCI_TIME_T now = get_mcci_time();

    while (now > m_bank_all.minimum_timeout()) m_bank_all.remove_minimum();

    while (now > m_bank_host.minimum_timeout()) m_bank_host.remove_minimum();

    while (now > m_bank_var.minimum_timeout()) m_bank_var.remove_minimum();

    while (now > m_bank_hostvar.minimum_timeout()) m_bank_hostvar.remove_minimum();

    while (now > m_bank_varrev.minimum_timeout()) m_bank_varrev.remove_minimum();
    
    return 1;
}

