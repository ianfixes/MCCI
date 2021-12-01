
#include <string.h>
#include <rhash/rhash.h> /* LibRHash interface */
#include "MCCIServer.h"

CMCCIServer* myServer;


int testSHA1()
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


int main(int argc, char* argv[])
{
    SMCCIServerSettings settings;
    settings.hashRemoteRequests = true;
    // these should be prime numbers because they become hash table sizes
    settings.maxLocalRequests = 101;
    settings.maxRemoteRequests = 199;

    myServer = new CMCCIServer(settings);


    testSHA1();
}
