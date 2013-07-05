
#pragma once

#include "MCCITypes.h"
#include <ostream>

/**
   This class provides all necessary socket functionality needed by the server
*/
class CMCCIServerNetworking
{

  public:

    // send a response to a production packet
    virtual void send_production_response(MCCI_CLIENT_ID_T client,
                                          const SMCCIAcceptancePacket* p) = 0;

    // send data
    virtual void send_data_to_client(MCCI_CLIENT_ID_T client,
                                     const SMCCIDataPacket *p) = 0;
    

    // send a request to be delivered to all clients
    virtual void forward_request(MCCI_CLIENT_ID_T requestor_id,
                                 const SMCCIRequestPacket* request) = 0;
    // FIXME: this will be a 0mq publish operation, queue name will be host.var.rev i think
};


class CMCCIServerNetworkingFake : public CMCCIServerNetworking
{
  protected:
    ostream* m_out;

    ostream& out() { return *m_out; }
    
  public:
    CMCCIServerNetworkingFake(ostream& outstream) : CMCCIServerNetworking()
    {
        this->m_out = &outstream;
    }

    virtual void send_production_response(MCCI_CLIENT_ID_T client,
                                          const SMCCIAcceptancePacket* p)
    {
        out() << "\nFAKENET Responding to client(" << client << ")'s production with " << *p;
    }

    virtual void send_data_to_client(MCCI_CLIENT_ID_T client,
                                     const SMCCIDataPacket* p)
    {
        out() << "\nFAKENET Giving client(" << client << ") some data: " << *p;
    }    
    
    virtual void forward_request(MCCI_CLIENT_ID_T requestor_id,
                                 const SMCCIRequestPacket* request)
    {
        out() << "\nFAKENET Forwarding client(" << requestor_id << ")'s request: " << *request;
    }
    
};
