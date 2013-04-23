
#if !defined(MCCI_PACKETS_H_INCLUDED)
#define MCCI_PACKETS_H_INCLUDED

typedef unsigned int MCCI_TIME_T;


typedef struct
{
    unsigned int NodeAddress;
    unsigned int VariableID;
    unsigned int Revision;
    char*        Payload;
    
} SMCCIDataPacket;


typedef struct
{
    unsigned int VariableID;
    unsigned int ResponseID;
    char*        Payload;

} SMCCIProductionPacket;


typedef struct
{
    unsigned int ResponseID;
    unsigned int Revision;

} SMCCIAcceptancePacket;


typedef struct
{
    MCCI_TIME_T  Timeout;
    unsigned int NodeAddress;
    unsigned int VariableID;
    unsigned int Revision;
    int          Quantity;
    // direction is implied in the sign of Quantity
    
} SMCCIRequestPacket;


typedef struct
{
    bool Accepted;   /// FIXME, maybe convert to int error code
    int  RequestsRemaining;

} SMCCIResponsePacket;

    
    
    



#endif // !defined(MCCI_PACKETS_H_INCLUDED)
