
#include "MCCIServer.h"
#include "MCCIRevisionSet.h"
#include "MCCISchema.h"

#include <string.h>
#include <sqlite3.h>
#include <stdio.h>

using namespace std;

CMCCIServer* myServer;
CMCCIServerNetworkingFake fake_networking(cerr); // FIXME: replace with real thing
CMCCITimeFake fake_time;


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

    try
    {
        schema = new CMCCISchema(schema_db);
        rs     = new CMCCIRevisionSet(rs_db, schema->get_cardinality(), schema->get_hash());
        
        // build settings struct
        SMCCIServerSettings settings;
        
        // these should be prime numbers because they become hash table sizes
        settings.max_local_requests = 101;
        settings.max_remote_requests = 199;
        settings.max_clients = 100;

        settings.bank_size_host = 20;
        settings.bank_size_var = 20;
        settings.bank_size_hostvar = 30;
        settings.bank_size_varrev_var = 100;
        settings.bank_size_varrev_rev = 20;
        settings.bank_size_remote_hostvar = 20;
        settings.bank_size_remote_rev = 20;
        
        // assign other objects
        settings.schema = schema;
        settings.revisionset = rs;

        
        myServer = new CMCCIServer((CMCCITime*)&fake_time,
                                   (CMCCIServerNetworking*)&fake_networking,
                                   settings);
        
        cleanup();
    }
    catch (std::bad_alloc ba)
    {
        printf("\nGot exception '%s'", ba.what());
    }
    catch (string s)
    {
        printf("\n\nGot error: %s\n\n", s.c_str());
    }
    catch (...)
    {
        printf("\n well... we caught some error");
    }


    // MAIN SERVER LOOP GOES HERE

    
    return 0;
    
}
