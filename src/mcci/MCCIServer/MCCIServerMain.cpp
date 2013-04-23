
#include "MCCIServer.h"
#include "MCCIRevisionSet.h"
#include "MCCISchema.h"

#include <string.h>
#include <sqlite3.h>
#include <stdio.h>

using namespace std;

CMCCIServer* myServer;


sqlite3* schema_db = NULL;
sqlite3* rs_db     = NULL;


void cleanup()
{
    sqlite3_close(schema_db);
    sqlite3_close(rs_db);
}


bool try_open_db(string file, sqlite3** db, int flags)
{
    int result;
    result = sqlite3_open_v2(file.c_str(), db, flags, NULL);
    if (SQLITE_OK != result)
    {
        fprintf(stderr, "\nCouldn't open '%s': '%s'", file.c_str(), sqlite3_errmsg(*db));
        return false;
    }
    return true;
}


int main(int argc, char* argv[])
{
    CMCCISchema* schema = NULL;
    CMCCIRevisionSet* rs = NULL;
    
    int result;

    if (!try_open_db("db.sqlite3", &schema_db, SQLITE_OPEN_READONLY))
    {
        cleanup();
        return 1;
    }
    if (!try_open_db("revisions.sqlite3", &rs_db, SQLITE_OPEN_READWRITE))
    {
        cleanup();
        return 1;
    }

    schema = new CMCCISchema(schema_db);
    rs     = new CMCCIRevisionSet(rs_db, "signature");

    //fprintf(stderr, "\nNull signature is '%s'", rs->getSignature().c_str());
    //rs->setSignature("Hello");
    //fprintf(stderr, "\nNull signature is '%s'", rs->getSignature().c_str());
            
    SMCCIServerSettings settings;
    // these should be prime numbers because they become hash table sizes
    settings.maxLocalRequests = 101;
    settings.maxRemoteRequests = 199;

    myServer = new CMCCIServer(settings);
    
    cleanup();
    return 0;
    
}
