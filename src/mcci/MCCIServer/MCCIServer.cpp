
#include "MCCIServer.h"

using namespace std;


//default constructor
CMCCIServer::CMCCIServer(CMCCITime* time,
                         CMCCIServerNetworking* networking,
                         SMCCIServerSettings settings) :
    m_time(time),
    m_networking(networking),
    m_working_set(settings.schema->get_cardinality(), NULL),
    m_bank_all(100, 1),
    m_bank_host(settings.max_clients, settings.bank_size_host),
    m_bank_var(settings.max_clients, settings.bank_size_var),
    m_bank_hostvar(settings.max_clients, settings.bank_size_hostvar),
    m_bank_remote(settings.max_clients, settings.bank_size_remote_hostvar, settings.bank_size_remote_rev),
    m_bank_varrev(settings.max_clients, settings.bank_size_varrev_var, settings.bank_size_varrev_rev)
{
    m_settings = settings;

    m_external_time = (NULL != m_time);
    if (!m_time)
    {
        m_time = (CMCCITime*) new CMCCITimeReal();
    }

}

//copy constructor
CMCCIServer::CMCCIServer(const CMCCIServer& rhs) :
    m_time(rhs.m_time),
    m_networking(rhs.m_networking),
    m_working_set(rhs.m_settings.schema->get_cardinality(), NULL),
    m_bank_all(100, 1),
    m_bank_host(rhs.m_settings.max_clients, rhs.m_settings.bank_size_host),
    m_bank_var(rhs.m_settings.max_clients, rhs.m_settings.bank_size_var),
    m_bank_hostvar(rhs.m_settings.max_clients, rhs.m_settings.bank_size_hostvar),
    m_bank_remote(rhs.m_settings.max_clients,
                  rhs.m_settings.bank_size_remote_hostvar,
                  rhs.m_settings.bank_size_remote_rev),
    m_bank_varrev(rhs.m_settings.max_clients,
                  rhs.m_settings.bank_size_varrev_var,
                  rhs.m_settings.bank_size_varrev_rev),
    m_settings(rhs.m_settings),
    m_external_time(rhs.m_external_time)
{
    return;
}


//destructor
CMCCIServer::~CMCCIServer()
{
    vector<SMCCIDataPacket*>::iterator it;
    for (it = m_working_set.begin(); it!= m_working_set.end(); ++it)
    {
        delete (*it);
    }

    // if we created it, destroy it.
    if (!m_external_time) delete m_time;
}


ostream& operator<<(ostream& out, const SMCCIServerSettings& rhs)
{
    return out 
        << "MCCIServer Settings:"
        << "\n\tNode Address:\t" << rhs.my_node_address
        << "\n\tMax local requests:\t" << rhs.max_local_requests
        << "\n\tMax remote requests:\t" << rhs.max_remote_requests
        << "\n\tMax Clients:\t" << rhs.max_clients
        << "\n\tBank size for host:\t" << rhs.bank_size_host
        << "\n\tBank size for var:\t" << rhs.bank_size_var
        << "\n\tBank size for host+var:\t" << rhs.bank_size_hostvar
        << "\n\tBank size for var/rev's var:\t" << rhs.bank_size_varrev_var
        << "\n\tBank size for var/rev's rev:\t" << rhs.bank_size_varrev_rev
        << "\n\tBank size for remote's host+var:\t" << rhs.bank_size_remote_hostvar
        << "\n\tBank size for remote's rev:\t" << rhs.bank_size_remote_rev
        ;

}


ostream& operator<<(ostream& out, const CMCCIServer& rhs)
{
    out << "MCCIServer Summary:"

        << "\n\tRequest Banks:"
        << "\n\t\t All:     " << rhs.m_bank_all
        << "\n\t\t Host:    " << rhs.m_bank_host
        << "\n\t\t Var:     " << rhs.m_bank_var
        << "\n\t\t HostVar: " << rhs.m_bank_hostvar
        << "\n\t\t Remote:  " << rhs.m_bank_remote
        << "\n\t\t VarRev:  " << rhs.m_bank_varrev
               ;

    LinearHash<MCCI_CLIENT_ID_T, bool> hits(100);

    
    out << "\n\tClients (with more than 1 open request):";
    // check promiscuous request bank for client matches
    for (AllRequestBank::subscriber_iterator it = rhs.m_bank_all.subscribers_begin(1);
         it != rhs.m_bank_all.subscribers_end(1); ++it)
    {
        hits[*it] = true;
    }

    // make output
    for (int i = 0; i < rhs.m_settings.max_clients; ++i)
    {
        //FIXME: maybe convert to client existence function
        int req_loc = rhs.m_settings.max_local_requests - rhs.client_free_requests_local(i);
        int req_rem = rhs.m_settings.max_remote_requests - rhs.client_free_requests_remote(i);

        if (req_loc || req_rem || hits[i])
        {
            out << "\n\t\t" << i << ":\t"
                << req_loc << " local requests, "
                << req_rem << " remote requests, "
                << (int)hits[i] << " promiscuous";
        }
    }
    
    
    out << "\n\tWorking Set:";
    for (int i = 0; i < rhs.m_settings.schema->get_cardinality(); ++i)
    {
        MCCI_VARIABLE_T var_id = rhs.m_settings.schema->variable_of_ordinal(i);
        if (rhs.is_in_working_set(var_id))
        {
            out << "\n\t\t" << rhs.m_settings.schema->name_of_variable(var_id);
        }
    }
    
    return out;
}


string CMCCIServer::summary()
{
    stringstream s;
    s << (*this);
    return s.str();
}


bool CMCCIServer::is_rejectable_request(const SMCCIRequestPacket* input) const
{
    return 0 < input->revision && (
        // requests with a sequence number but no variable
        (0 == input->node_address && 0 == input->variable_id)
        // requests with no host but yes variable
        || (MCCI_HOST_ANY == input -> node_address && 0 < input->variable_id)); 
}


void CMCCIServer::process_request(MCCI_CLIENT_ID_T requestor_id,
                                  const SMCCIRequestPacket* input,
                                  SMCCIResponsePacket* response)
{
    // pre-fill these because we use them for flags right now
    response->requests_remaining_local  = client_free_requests_local(requestor_id);
    response->requests_remaining_remote = client_free_requests_remote(requestor_id);
    
    // process rejections first -- even before timeouts -- so that bad packets always reject
    if (is_rejectable_request(input))
    {
        response->accepted = false;
        return set_free_requests(response, requestor_id);
    }
    response->accepted = true;     // all requests are (or should be) OK after this
    
    // we allow packets that are timed out just in case they replace existing requests
    
    // if subscribing to ALL nodes
    if (MCCI_HOST_ANY == input->node_address) 
    {
        // drop request silently if we are full
        if (!response->requests_remaining_remote) return set_free_requests(response, requestor_id);
        
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

        //response->requests_remaining_remote = client_free_requests_remote(requestor_id);
        return set_free_requests(response, requestor_id);
    }
    
    // subscribing to one node that may be local or remote
    if (0 == input->variable_id && 0 == input->revision)
    {
        // drop request silently if we are full
        if (!response->requests_remaining_remote) return set_free_requests(response, requestor_id);
        
        // fill in real address of host (if wildcarded)
        MCCI_NODE_ADDRESS_T real_address;
        real_address = input->node_address ? input->node_address : m_settings.my_node_address;
        subscribe_to_host(requestor_id, input->timeout, real_address);

        return set_free_requests(response, requestor_id);
    }
    
    process_forwardable_request(requestor_id, input, response);
}


void CMCCIServer::process_forwardable_request(MCCI_CLIENT_ID_T requestor_id,
                                              const SMCCIRequestPacket* input,
                                              SMCCIResponsePacket* response)
{
    //TODO: assert input->variable_id != 0
    //TODO: assert node_address != ALL
    // response->requests_remaining_* have already been initialized
    
    bool is_for_me = is_my_address(input->node_address);
    bool do_forward = !is_for_me;

    if (!is_for_me && 0 == input->revision)
    {
        // silently drop if we have no more requests
        if (response->requests_remaining_remote) 
        {
            subscribe_to_host_var(requestor_id, input->timeout, input->node_address, input->variable_id);
            m_networking->forward_request(requestor_id, input);
        }

        return set_free_requests(response, requestor_id);
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
    {
        if (1 == direction)
            firstrev = input->revision;
        else
            firstrev = input->revision + quantity - limit;
    }
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


    /*
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
    */
    
    // we are forwarding a request that might be for more packets than available
    //  subscription slots.  i think this is ok, since the send ordering is preserved
    //  and it's possible that the slots will clear (for re-request) before the later
    //  packets arrive.

    if (do_forward) m_networking->forward_request(requestor_id, input);

    return set_free_requests(response, requestor_id);

}


void CMCCIServer::subscribe_promiscuous(MCCI_CLIENT_ID_T client_id, MCCI_TIME_T timeout)
{
    m_bank_all.add(1, client_id, timeout);
}

void CMCCIServer::subscribe_to_host(MCCI_CLIENT_ID_T client_id,
                                    MCCI_TIME_T timeout,
                                    MCCI_NODE_ADDRESS_T node_address)
{
    m_bank_host.add(node_address, client_id, timeout);
}

void CMCCIServer::subscribe_to_variable(MCCI_CLIENT_ID_T client_id,
                                        MCCI_TIME_T timeout,
                                        MCCI_VARIABLE_T variable_id)
{
    m_bank_var.add(variable_id, client_id, timeout);
}

void CMCCIServer::subscribe_to_host_var(MCCI_CLIENT_ID_T client_id,
                                        MCCI_TIME_T timeout,
                                        MCCI_NODE_ADDRESS_T host,
                                        MCCI_VARIABLE_T variable_id)
{
    HostVarPair hv;
    hv.host = host;
    hv.var = variable_id;
    m_bank_hostvar.add(hv, client_id, timeout);
}

void CMCCIServer::subscribe_specific_remote(MCCI_CLIENT_ID_T client_id,
                                            MCCI_TIME_T timeout,
                                            MCCI_NODE_ADDRESS_T node_address,
                                            MCCI_VARIABLE_T variable_id,
                                            MCCI_REVISION_T revision)
{
    HostVarRevTuple hvr;
    hvr.host = node_address;
    hvr.var = variable_id;
    hvr.rev = revision;
    m_bank_remote.add(hvr, client_id, timeout);
}


void CMCCIServer::subscribe_specific(MCCI_CLIENT_ID_T client_id,
                                     MCCI_TIME_T timeout,
                                     MCCI_VARIABLE_T variable_id,
                                     MCCI_REVISION_T revision)
{
    VarRevPair vr;
    vr.var = variable_id;
    vr.rev = revision;
    m_bank_varrev.add(vr, client_id, timeout);
}



bool CMCCIServer::bank_contains_variable(MCCI_CLIENT_ID_T client_id,
                                         MCCI_VARIABLE_T variable_id) const
{
    return m_bank_var.contains(variable_id, client_id);
}
    

bool CMCCIServer::bank_contains_host(MCCI_CLIENT_ID_T client_id, 
                                     MCCI_NODE_ADDRESS_T node_address) const
{
    return m_bank_host.contains(node_address, client_id);    
}


bool CMCCIServer::bank_contains_host_var(MCCI_CLIENT_ID_T client_id,
                                         MCCI_NODE_ADDRESS_T node_address,
                                         MCCI_VARIABLE_T variable_id) const
{
    HostVarPair hv;
    hv.host = node_address;
    hv.var = variable_id;
    return m_bank_hostvar.contains(hv, client_id);
}


bool CMCCIServer::bank_contains_specific(MCCI_CLIENT_ID_T client_id,
                                         MCCI_VARIABLE_T variable_id,
                                         MCCI_REVISION_T revision) const
{
    VarRevPair vr;
    vr.var = variable_id;
    vr.rev = revision;
    return m_bank_varrev.contains(vr, client_id);
}

bool CMCCIServer::bank_contains_specific_remote(MCCI_CLIENT_ID_T client_id,
                                                MCCI_NODE_ADDRESS_T node_address,
                                                MCCI_VARIABLE_T variable_id,
                                                MCCI_REVISION_T revision) const
{
    HostVarRevTuple hvr;
    hvr.host = node_address;
    hvr.var = variable_id;
    hvr.rev = revision;
    return m_bank_remote.contains(hvr, client_id);

}
    

void CMCCIServer::process_production(MCCI_CLIENT_ID_T provider_id,
                                     const SMCCIProductionPacket* input,
                                     SMCCIAcceptancePacket* output)
{
    // hit the revisionset for the revision id
    MCCI_REVISION_T rev = m_settings.revisionset->inc_revision(input->variable_id);
    
    // fill in the fields of the data and acceptance packets
    SMCCIDataPacket* dp = new SMCCIDataPacket();
    dp->node_address = m_settings.my_node_address;
    dp->variable_id  = input->variable_id;
    dp->revision     = rev;
    dp->payload      = input->payload;

    output->response_id = input->response_id;
    output->revision    = rev;
    
    // add the packet to the working set (will free anything that was there)
    set_working_variable(input->variable_id, dp);

    // call process_data with the new packet
    process_data(provider_id, dp);

    if (output->response_id)
    {
        this->m_networking->send_production_response(provider_id, output);
    }
}


void CMCCIServer::process_data(MCCI_CLIENT_ID_T provider_id, const SMCCIDataPacket* input)
{

    // create linear hash
    LinearHash<MCCI_CLIENT_ID_T, bool> hits(100);

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

    HostVarRevTuple hvr;
    hvr.host = input->node_address;
    hvr.var  = input->variable_id;
    hvr.rev  = input->revision;
    for (RemoteRevisionRequestBank::subscriber_iterator it = m_bank_remote.subscribers_begin(hvr);
         it != m_bank_remote.subscribers_end(hvr); ++it)
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
    for (LinearHash<MCCI_CLIENT_ID_T, bool>::iterator it = hits.begin();
         it != hits.end(); ++it)
    {
        m_networking->send_data_to_client(it->first, input);
    }


    //FIXME: send ack to provider_id?
    enforce_fulfillment(input);

}


unsigned int CMCCIServer::client_free_requests_local(MCCI_CLIENT_ID_T client_id) const
{
    // all outstanding requests for this client in all local banks
    return m_settings.max_local_requests - (
        0 // m_bank_all doesn't count
        + m_bank_varrev.get_outstanding_request_count(client_id)
        ); 
}
    
unsigned int CMCCIServer::client_free_requests_remote(MCCI_CLIENT_ID_T client_id) const
{
    // all outstanding requests for this client in all remote banks
    return m_settings.max_remote_requests - (
        0 // m_bank_all doesn't count
        + m_bank_host.get_outstanding_request_count(client_id)
        + m_bank_var.get_outstanding_request_count(client_id)
        + m_bank_hostvar.get_outstanding_request_count(client_id)
        + m_bank_remote.get_outstanding_request_count(client_id)
        );  
}

void CMCCIServer::enforce_fulfillment(const SMCCIDataPacket* delivered)
{
    if (is_my_address(delivered->node_address))
    {
        VarRevPair vr;
        vr.var = delivered->variable_id;
        vr.rev = delivered->revision;

        if (m_bank_varrev.get_by_pq(vr)) m_bank_varrev.remove_by_key(vr);
    }
    else
    {
        HostVarRevTuple hvr;
        hvr.host = delivered->node_address;
        hvr.var = delivered->variable_id;
        hvr.rev = delivered->revision;
        
        if (m_bank_remote.get_by_pq(hvr)) m_bank_remote.remove_by_key(hvr);
    }
}

void CMCCIServer::enforce_timeouts()
{
    MCCI_TIME_T now = m_time->now();

    //TODO: if we have a lot of removals, defer some of them until later?
    // in other words take only n of k removals if n < k, but for every deferal
    // if k > last_n then take more than n.
    
    while (!m_bank_all.empty() && now > m_bank_all.minimum_timeout())
        m_bank_all.remove_minimum();

    while (!m_bank_host.empty() && now > m_bank_host.minimum_timeout())
        m_bank_host.remove_minimum();

    while (!m_bank_var.empty() && now > m_bank_var.minimum_timeout())
        m_bank_var.remove_minimum();

    while (!m_bank_hostvar.empty() && now > m_bank_hostvar.minimum_timeout())
        m_bank_hostvar.remove_minimum();

    while (!m_bank_remote.empty() && now > m_bank_remote.minimum_timeout())
        m_bank_remote.remove_minimum();
    
    while (!m_bank_varrev.empty() && now > m_bank_varrev.minimum_timeout())
        m_bank_varrev.remove_minimum();
}

