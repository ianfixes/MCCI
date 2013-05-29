
#include "MCCISchema.h"
#include <string>
using namespace std;

void CMCCISchema::load(sqlite3* schema_db)
{
    unsigned int cardinality = load_cardinality(schema_db);
    
    m_ordinality.resize_nearest_prime(cardinality);
    m_name.resize(cardinality);
    m_variable.resize(cardinality);
    
    // variables for calculating hash value
    unsigned char md[SHA_DIGEST_LENGTH];    
    SHA_CTX context;
    int init_success = SHA1_Init(&context);
    int update_success;
        
    // variables for sqlite reading
    sqlite3_stmt* stmt = NULL;
    int result;
    char data[512];
    int datalen;
    MCCI_VARIABLE_T var_id;
    string var_name;
    
    result = sqlite3_prepare_v2(schema_db,
                                "select var_id, name from var where enabled <> 0",
                                -1, &stmt, 0);

    if (result) throw string("Loading of data filed FIXME: result");

    // this is where we iterate through the schema db
    for (int i = 0; ; i++)
    {
        result = sqlite3_step(stmt);

        if (SQLITE_DONE == result) break;

        if (SQLITE_ROW != result) throw string("Something bad");

        var_id = (MCCI_VARIABLE_T) sqlite3_column_int(stmt, 0);
        var_name = string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
        
        // set lookup values
        m_ordinality[var_id] = i;
        m_variable[i] = var_id;
        m_name[i] = var_name;

        // update hash
        datalen = snprintf(data, 512, "%d\t%s\n", var_id, var_name.c_str());
        if (512 < datalen) throw string("Got a line that was too long");
        update_success = SHA1_Update(&context, data, datalen);
    }
    
    // finalize the hash value
    int final_success = SHA1_Final(md, &context);    
    if (init_success + update_success + final_success)
        init_success = 0; // TODO: remove this line which is to suppress warning
    
    string ret(md, md + SHA_DIGEST_LENGTH - 1);
    this->m_hashval = ret;
        
}

unsigned int CMCCISchema::load_cardinality(sqlite3* schema_db)
{
    unsigned int cardinality;
    int result;
    // run a count query to find out how many entries we need (i.e. cardinality)
    sqlite3_stmt* stmt = NULL;

    // execute statement
    result = sqlite3_prepare_v2(schema_db,
                                "select count(*) from var where enabled <> 0",
                                -1, &stmt, 0);
    if (result) throw string("select failed, FIXME use result");

    result = sqlite3_step(stmt);
    if (SQLITE_ROW != result) throw string("Couldn't read count for some reason");

    cardinality = (unsigned int) sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return cardinality;
}
