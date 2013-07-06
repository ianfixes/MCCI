
#include "MCCISchema.h"
#include <string>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

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
    int final_success;
    char hash[SHA_DIGEST_LENGTH * 2];
    
    // variables for sqlite reading
    sqlite3_stmt* stmt = NULL;
    int result;
    char data[512];
    int datalen;
    MCCI_VARIABLE_T var_id;
    string var_name;
    long var_pbuf;
    long var_unit;
    
    result = sqlite3_prepare_v2(schema_db,
                                "select var_id, name, protobuf_id, unit "
                                "from var where enabled <> 0",
                                -1, &stmt, 0);

    if (result) throw string("Loading of data failed FIXME: result");


    // this is where we iterate through the schema db
    for (unsigned int i = 0; ; i++)
    {
        result = sqlite3_step(stmt);

        if (SQLITE_DONE == result) break;

        if (SQLITE_ROW != result) throw string("Something bad");

        var_id = (MCCI_VARIABLE_T) sqlite3_column_int(stmt, 0);
        var_name = string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
        var_pbuf = sqlite3_column_int(stmt, 2);
        var_unit = sqlite3_column_int(stmt, 3);
        
        // set lookup values
        m_ordinality[var_id] = i;
        m_variable[i] = var_id;
        m_name[i] = var_name;

        // update hash
        datalen = snprintf(data, 512, "%d\t%s\t%ld\t%ld\n",
                           var_id, var_name.c_str(), var_pbuf, var_unit);
        if (512 < datalen) throw string("Got a line that was too long");
        update_success = SHA1_Update(&context, data, datalen);
    }

    sqlite3_finalize(stmt);

    // finalize the hash value
    final_success = SHA1_Final(md, &context);    

    if (3 > init_success + update_success + final_success)
    {
        fprintf(stderr, "\nSHA1 errors: %d %d %d", init_success, update_success, final_success);
        throw string("SHA1 process had a zero return code somewhere");
    }

    b64_encode(md, hash, SHA_DIGEST_LENGTH, SHA_DIGEST_LENGTH * 2);

    this->m_hashval = string(hash);
}

// base64 encode function using the openssl library
// http://doctrina.org/Base64-With-OpenSSL.html
void CMCCISchema::b64_encode(unsigned char* in,
                             char* out,
                             unsigned int in_len,
                             unsigned int out_len)
{
    FILE* out_file = fmemopen(out, out_len, "w"); // fake file in memory

    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_new_fp(out_file, BIO_NOCLOSE);
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
    BIO_write(bio, in, in_len);
    BIO_flush(bio);
    
    b64 = BIO_pop(bio);
    BIO_free(bio);
    BIO_free(b64);
    bio = NULL;
    b64 = NULL;
    
    fclose(out_file);
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
