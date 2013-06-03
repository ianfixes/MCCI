#pragma once

#include <boost/cstdint.hpp>

#include <ostream>

using namespace std;

typedef uint16_t MCCI_VARIABLE_T;
typedef uint16_t MCCI_NODE_ADDRESS_T;
typedef uint32_t MCCI_REVISION_T;
typedef uint16_t MCCI_CLIENT_ID_T;
typedef uint32_t MCCI_TIME_T;

typedef char* MCCI_PAYLOAD_T; // FIXME, need len or different data type here

#define MCCI_HOST_ANY ((uint16_t) -1)


typedef struct
{
    MCCI_NODE_ADDRESS_T node_address;
    MCCI_VARIABLE_T     variable_id;
    MCCI_REVISION_T     revision;
    MCCI_PAYLOAD_T      payload; 
    
} SMCCIDataPacket;


typedef struct
{
    MCCI_VARIABLE_T variable_id;
    unsigned int    response_id;
    MCCI_PAYLOAD_T  payload; 

} SMCCIProductionPacket;


typedef struct
{
    unsigned int    response_id;
    MCCI_REVISION_T revision;

} SMCCIAcceptancePacket;


typedef struct
{
    MCCI_TIME_T         timeout;
    MCCI_NODE_ADDRESS_T node_address;
    MCCI_VARIABLE_T     variable_id;
    MCCI_REVISION_T     revision;
    int                 quantity;
    // direction is implied in the sign of Quantity
    
} SMCCIRequestPacket;


typedef struct
{
    bool          accepted;   /// FIXME, maybe convert to int error code
    unsigned int  requests_remaining_local;
    unsigned int  requests_remaining_remote;

} SMCCIResponsePacket;


// ostream functions

inline ostream& operator<<(ostream& out, const SMCCIDataPacket& rhs)
{
    return out
        << "(node_address: " << rhs.node_address << ", "
        << "variable_id: " << rhs.variable_id << ", "
        << "revision: " << rhs.revision << ", "
        << "payload: <fixme: payload len>)";
}

inline ostream& operator<<(ostream& out, const SMCCIProductionPacket& rhs)
{
    return out
        << "(variable_id: " << rhs.variable_id << ", "
        << "response_id: " << rhs.response_id << ", "
        << "payload: <fixme: payload len>)";
}

inline ostream& operator<<(ostream& out, const SMCCIAcceptancePacket& rhs)
{
    return out
        << "(response_id: " << rhs.response_id << ", "
        << "revision: " << rhs.revision << ")";
}
    
inline ostream& operator<<(ostream& out, const SMCCIRequestPacket& rhs)
{
    return out
        << "(timeout: " << rhs.timeout << ", "
        << "node_address: " << rhs.node_address << ", "
        << "variable_id: " << rhs.variable_id << ", "
        << "revision: " << rhs.revision << ", "
        << "quantity: " << rhs.quantity << ")";
}

inline ostream& operator<<(ostream& out, const SMCCIResponsePacket& rhs)
{
    return out
        << "(accepted: " << rhs.accepted << ", "
        << "requests_remaining_local: " << rhs.requests_remaining_local << ", "
        << "requests_remaining_remote: " << rhs.requests_remaining_remote << ")";
}



