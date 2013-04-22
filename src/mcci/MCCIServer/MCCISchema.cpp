
#include "MCCISchema.h"
#include <string.h>
using namespace std;

CMCCISchema::CMCCISchema()
{
    rhash_library_init();
    m_db = NULL;
}



int foo()
{
    const char* msg = "message digest";
    unsigned char digest[64];
    char output[130];

    rhash_library_init(); /* initialize static data */

    int res = rhash_msg(RHASH_SHA1, msg, strlen(msg), digest);
    if(res < 0) {
        fprintf(stderr, "hash calculation error\n");
        return 1;
    }

    /* convert binary digest to hexadecimal string */
    rhash_print_bytes(output, digest, rhash_get_digest_size(RHASH_SHA1),
                      (RHPR_HEX | RHPR_UPPERCASE));

    printf("%s (\"%s\") = %s\n", rhash_get_name(RHASH_SHA1), msg, output);
    return 0;
}
