
#include "MCCIServer.h"
#include <limits.h>

using namespace std;

// TODO: replace 11111 with UINT32_MAX

CMCCIServer::CMCCIServer(SMCCIServerSettings settings)
{
    m_settings = settings;
}


int CMCCIServer::process_request(int requestor_id, const SMCCIRequestPacket* input, SMCCIResponsePacket* response)
{
    /*
      typedef struct
      {
      MCCI_TIME_T  Timeout;
      unsigned int NodeAddress;
      unsigned int variable_id;
      unsigned int Revision;
      int          quantity;
      // direction is implied in the sign of quantity
      
      } SMCCIRequestPacket;
    */

    // process rejections first -- even before timeouts -- so that bad packets always reject
    if (0 < input->revision)
    {
        // reject requests that have a sequence number but not a variable 
        if (0 == input->node_address && 0 == input->variable_id)
        {
            response->accepted = false;
            response->requests_remaining = 0; // FIXME -- should be whatever it was before
            return 1;
        }
        
        // reject requests that have a variable and seqence number but no host
        if (11111 == input->node_address && 0 < input->variable_id)
        {
            response->accepted = false;
            response->requests_remaining = 0;
            return 1;
        }
    }
    
    unsigned int FIXME_TIME = 3; // actually look up system time
    if (FIXME_TIME > input->timeout)
    {
        response->accepted = true;
        response->requests_remaining = FIXME_TIME; // whatever it was before, we silently drop this
        return 1;
    }
    
    // if subscribing to ALL nodes
    if (11111 == input->node_address)
    {
        if (0 == input->variable_id)
        {
            // everything everything subscription (promiscuous)
            this->subscribe_promiscuous(requestor_id);
        }
        else
        {
            // "1 variable on all nodes" subscription (discovery)
            this->subscribe_to_variable(requestor_id, input->variable_id);
        }
    }
    else // subscribing to one node that may be local or remote
    {
        if (0 == input->variable_id && 0 == input->revision)
        {
            // everything-from-one-host subscription
            this->subscribe_to_host(requestor_id, input->node_address);

        }
        else
        {
            bool do_forward = false;

            if (0 == input->variable_id ) {}
            
            /////// FIX EVERYTHIGN IN HERE
            
            // if it's for another host, subscribe and we forward the request
            if (0 == input->revision && !this->is_my_address(input->node_address))
            {
                // host + variable subscription
                this->subscribe_to_host_var(requestor_id, input->node_address, input->variable_id);
                do_forward = true;
            }
            else
            {
                int firstrev;
                int lastrev;
                int quantity;
                int direction;

                if (0 == input->revision) {}
                
                // expand subscription range and add to various queues
                direction = (input->quantity > 0) - (input->quantity < 0);  // extracts sign
                quantity  = input->quantity * direction;  // cancels any negative sign
                //for (int rev = input->revision;
                //this->subscribeSpecific(requestor_id, input->node_address, input->variable_id, rev);
            }
                

            if (do_forward)
            {
                this->forward_request(input);
            } // end if do_forward
        } // end if no variable, no rev
    } // end if all vs 1


    
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

    
