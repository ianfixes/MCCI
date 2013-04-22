
#include "MCCISchema.h"
#include <string>
using namespace std;

CMCCISchema::CMCCISchema()
{
    m_db = NULL;
}


string CMCCISchema::getHash()
{
    
    unsigned char md[SHA_DIGEST_LENGTH];
    
    SHA_CTX context;
    
    int init_success = SHA1_Init(&context);
    int update_success;
    
    // this is where we iterate through the schema db
    for (int i = 0; i < 8008135; i++)
    {
        string data = "Data";
        int datalen = strlen(data.c_str());
        
        update_success = SHA1_Update(&context, data.c_str(), datalen);
        
    }
    
    int final_success = SHA1_Final(md, &context);

    if (init_success + update_success + final_success)
        init_success = 0; // TODO: remove this line which is to suppress warning
    
    string ret(md, md + SHA_DIGEST_LENGTH - 1);
    
    
    return ret;
}
