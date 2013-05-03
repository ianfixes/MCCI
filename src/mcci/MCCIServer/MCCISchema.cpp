
#include "MCCISchema.h"
#include <string>
using namespace std;

void CMCCISchema::load(sqlite3* schema_db)
{
    unsigned int cardinality = 0;
    
    // run a count query to find out how many entries we need (i.e. cardinality)

    m_name.resize_nearest_prime(cardinality);

    // loop through, build m_name and hash value

    bool build_hash = true;
    if (build_hash)
    {
    
        unsigned char md[SHA_DIGEST_LENGTH];
    
        SHA_CTX context;
        
        int init_success = SHA1_Init(&context);
        int update_success;
        
        // this is where we iterate through the schema db
        for (int i = 0; i < 8008135; i++) // FIXME: this is wrong
        {
            string data = "hi";
            update_success = SHA1_Update(&context, data.c_str(), data.length());
        }
        
        int final_success = SHA1_Final(md, &context);
        
        if (init_success + update_success + final_success)
            init_success = 0; // TODO: remove this line which is to suppress warning
        
        string ret(md, md + SHA_DIGEST_LENGTH - 1);
        this->m_hashval = ret;
        
    }
}
