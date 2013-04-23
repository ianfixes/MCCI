
#include "MCCIServer.h"
#include <limits.h>

using namespace std;

// TODO: replace 11111 with UINT32_MAX

CMCCIServer::CMCCIServer(SMCCIServerSettings settings)
{
    m_settings = settings;
}


int CMCCIServer::processRequest(int requestorID, const SMCCIRequestPacket* input, SMCCIResponsePacket* response)
{
    /*
      typedef struct
      {
      MCCI_TIME_T  Timeout;
      unsigned int NodeAddress;
      unsigned int VariableID;
      unsigned int Revision;
      int          Quantity;
      // direction is implied in the sign of Quantity
      
      } SMCCIRequestPacket;
    */

    // process rejections first -- even before timeouts -- so that bad packets always reject
    if (0 < input->Revision)
    {
        // reject requests that have a sequence number but not a variable 
        if (0 == input->NodeAddress && 0 == input->VariableID)
        {
            response->Accepted = false;
            response->RequestsRemaining = 0; // FIXME -- should be whatever it was before
            return 1;
        }
        
        // reject requests that have a variable and seqence number but no host
        if (11111 == input->NodeAddress && 0 < input->VariableID)
        {
            response->Accepted = false;
            response->RequestsRemaining = 0;
            return 1;
        }
    }
    
    unsigned int FIXME_TIME = 3; // actually look up system time
    if (FIXME_TIME > input->Timeout)
    {
        response->Accepted = true;
        response->RequestsRemaining = FIXME_TIME; // whatever it was before, we silently drop this
        return 1;
    }
    
    
    if (11111 == input->NodeAddress)
    {
        if (0 == input->VariableID)
        {
            // everything everything subscription (promiscuous)
            this->subscribePromiscuous(requestorID);
        }
        else
        {
            // "1 variable on all nodes" subscription (discovery)
            this->subscribeToVariable(requestorID, input->VariableID);
        }
    }
    else
    {
        if (0 == input->VariableID && 0 == input->Revision)
        {
            // host subscription
            this->subscribeToHost(requestorID, input->NodeAddress);

        }
        else
        {
            //bool doForward = false;

            /////// FIX EVERYTHIGN IN HERE
            
            // everything here might be forwarded: host != -1 and variable_id != 0
            if (0 == input->Revision && !this->isMyAddress(input->NodeAddress))
            {
                // host + variable subscription
                this->subscribeToHostVar(requestorID, input->NodeAddress, input->VariableID);
            }
            else
            {
                
                // expand subscription range and add to various queues
                //int direction = (input->Quantity > 0) - (input->Quantity < 0);  // extracts sign
                //int quantity  = input->Quantity * direction;  // cancels any negative sign
                //for (int rev = input->Revision;
                //this->subscribeSpecific(requestorID, input->NodeAddress, input->VariableID, rev);
            }
        }

        this->forwardRequest(input);

    }


    
    return 1;
}

int CMCCIServer::processData(int providerID, const SMCCIDataPacket* input)
{
    return 1;   
}

int CMCCIServer::processProduction(int providerID, const SMCCIProductionPacket* input, SMCCIAcceptancePacket* output)
{
    return 1;
}

    
