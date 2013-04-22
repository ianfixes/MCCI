
#include "MCCIServer.h"
#include <rhash/rhash.h>

CMCCIServer* myServer;



int main(int argc, char* argv[])
{
    SMCCIServerSettings settings;
    settings.hashRemoteRequests = true;
    // these should be prime numbers because they become hash table sizes
    settings.maxLocalRequests = 101;
    settings.maxRemoteRequests = 199;

    myServer = new CMCCIServer(settings);

}
