
#if !defined(MCCI_PACKETS_H_INCLUDED)
#define MCCI_PACKETS_H_INCLUDED

typedef unsigned int MCCI_TIME_T;


typedef struct
{
    unsigned int node_address;
    unsigned int variable_id;
    unsigned int revision;
    char*        payload;
    
} SMCCIDataPacket;


typedef struct
{
    unsigned int variable_id;
    unsigned int response_id;
    char*        payload;

} SMCCIProductionPacket;


typedef struct
{
    unsigned int response_id;
    unsigned int revision;

} SMCCIAcceptancePacket;


typedef struct
{
    MCCI_TIME_T  timeout;
    unsigned int node_address;
    unsigned int variable_id;
    unsigned int revision;
    int          quantity;
    // direction is implied in the sign of Quantity
    
} SMCCIRequestPacket;


typedef struct
{
    bool accepted;   /// FIXME, maybe convert to int error code
    int  requests_remaining;

} SMCCIResponsePacket;

    
    
    



#endif // !defined(MCCI_PACKETS_H_INCLUDED)
