
#include "MCCIServer.h"
#include "MCCIRevisionSet.h"
#include "MCCISchema.h"

#include <string.h>
#include <sqlite3.h>
#include <iostream>

using namespace std;

CMCCIServer* my_server;


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
    cerr << "\nOpening sqlite3 db (" <<  file.c_str() << ")...";
    result = sqlite3_open_v2(file.c_str(), db, flags, NULL);
    if (SQLITE_OK != result)
    {
        cerr << "Couldn't open '" << file << "': '" << sqlite3_errmsg(*db) << "'";
        return false;
    }
    cerr << "OK";
    return true;
}


int main(int argc, char* argv[])
{
    CMCCISchema* schema = NULL;
    CMCCIRevisionSet* rs = NULL;
    
    if (!try_open_db("../../../db.sqlite3", &schema_db, SQLITE_OPEN_READONLY))
    {
        cleanup();
        return 1;
    }
    if (!try_open_db("../../../revisions.sqlite3", &rs_db, SQLITE_OPEN_READWRITE))
    {
        cleanup();
        return 1;
    }

    try
    {
        cerr << "\nCreating schema object...";
        schema = new CMCCISchema(schema_db);
        cerr << "OK " << schema;

        cerr << "\nCreating revisionset object...";
        rs     = new CMCCIRevisionSet(rs_db, schema->get_cardinality(), "signature");
        cerr << "OK";
            
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

        cerr << "\nCreating server instance...";
        my_server = new CMCCIServer(settings);
        cerr << "OK";
        
        cleanup();
    }
    catch (std::bad_alloc ba)
    {
        cerr << "\nGot exception '" <<  ba.what() << "'";
    }
    catch (string s)
    {
        cerr << "\n\nGot error: " << s << "\n\n";
    }
    catch (...)
    {
        cerr << "\n well... we caught some error";
    }

    cerr << "\n---------------- LET THE TESTING BEGIN\n";
    
    cerr << "\n" << *my_server;

    MCCI_CLIENT_ID_T myclient_id = 37;
    /*
    my_server->process_request(myclient_id
                               const SMCCIRequestPacket* input,
                               SMCCIResponsePacket* response)
    */
    cerr << "\n" << *my_server;
    

        
    cerr << "\n\n";
    return 0;
    
}
