#pragma once

#include "MCCITime.h"

typedef unsigned int MCCI_VARIABLE_T;
typedef unsigned int MCCI_NODE_ADDRESS_T;
typedef unsigned int MCCI_REVISION_T;
typedef unsigned int MCCI_CLIENT_ID_T;

typedef struct
{
    MCCI_NODE_ADDRESS_T node_address;
    MCCI_VARIABLE_T     variable_id;
    MCCI_REVISION_T     revision;
    char*               payload;
    
} SMCCIDataPacket;


typedef struct
{
    MCCI_VARIABLE_T variable_id;
    unsigned int    response_id;
    char*           payload;

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


