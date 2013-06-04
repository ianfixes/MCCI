
#include "MCCIServer.h"
#include "MCCIRevisionSet.h"
#include "MCCISchema.h"
#include "MCCITime.h"

#include <string.h>
#include <sqlite3.h>
#include <iostream>
#include <assert.h>

using namespace std;

CMCCITimeFake fake_time;

sqlite3* schema_db = NULL;
sqlite3* rs_db     = NULL;

CMCCISchema* schema = NULL;
CMCCIRevisionSet* rs = NULL;

CMCCIServer* my_server;


void cleanup()
{
    sqlite3_close(schema_db);
    sqlite3_close(rs_db);

    delete my_server;
    delete schema;
    delete rs;
}


bool try_open_db(bool debug, string file, sqlite3** db, int flags)
{
    int result;
    if (debug) cerr << "\nOpening sqlite3 db (" <<  file.c_str() << ")...";
    result = sqlite3_open_v2(file.c_str(), db, flags, NULL);
    if (SQLITE_OK != result)
    {
        cerr << "Couldn't open '" << file << "': '" << sqlite3_errmsg(*db) << "'";
        return false;
    }
    if (debug) cerr << "OK";
    return true;
}


int init(bool debug)
{
    
    if (!try_open_db(debug, "../../../db.sqlite3", &schema_db, SQLITE_OPEN_READONLY))
    {
        cleanup();
        return 1;
    }
    if (!try_open_db(debug, "../../../revisions.sqlite3", &rs_db, SQLITE_OPEN_READWRITE))
    {
        cleanup();
        return 1;
    }

    try
    {
        if (debug) cerr << "\nCreating schema object...";
        schema = new CMCCISchema(schema_db);
        if (debug) cerr << "OK " << schema;

        if (debug) cerr << "\nCreating revisionset object...";
        rs     = new CMCCIRevisionSet(rs_db, schema->get_cardinality(), "signature");
        if (debug) cerr << "OK";
            
        // build settings struct
        SMCCIServerSettings settings;
        
        // these should be prime numbers because they become hash table sizes
        settings.my_node_address = 5;
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

        if (debug) cerr << "\nCreating server instance...";
        my_server = new CMCCIServer((CMCCITime*)&fake_time, settings);
        if (debug) cerr << "OK";
        
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


    return 0;
}

// generalized do_test function
int do_test(string name, int (*test_fn)(void))
{
    if (int init_ret = init(false))
    {
        cerr << "\ninit() got code " << init_ret << "; exiting\n\n";
        cleanup();
        return init_ret;
    }

    cerr << "\n\n---------------- LET THE TESTING BEGIN: " << name << "\n";
    if (int test_ret = test_fn())
    {
        cerr << "\ntest_fn '" << name << "' got code " << test_ret << "; exiting\n\n";
        cleanup();
        return test_ret;
    }

    cerr << "\n -- Test " << name << " done ";
    cleanup();
    cerr << "and cleaned up";
    
}

int test0()
{
    cerr << "\nEmpty server\n" << *my_server;
    return 0;
}

int test_rb_all()
{
    MCCI_CLIENT_ID_T myclient_id = 37;
    SMCCIServerSettings settings = my_server->get_settings();
    fake_time.set_now(12344);
    
    SMCCIRequestPacket request;
    request.timeout = fake_time.now();
    request.node_address = MCCI_HOST_ANY;
    request.variable_id = 0;
    request.revision = 0;
    request.quantity = 1;

    SMCCIResponsePacket response;
    response.accepted = false;
    response.requests_remaining_local = 0;
    response.requests_remaining_remote = 0;

    cerr << "\nTesting request acceptance";
    request.timeout = fake_time.now() + 1;
    my_server->process_request(myclient_id, &request, &response);
    assert(response.accepted);
    // requests for ALL don't count against totals!
    assert(settings.max_local_requests == response.requests_remaining_local);
    assert(settings.max_remote_requests == response.requests_remaining_remote);

    cerr << "\nenforcing timeouts";
    my_server->enforce_timeouts();
    
    cerr << "\nTesting request alteration";
    request.timeout = fake_time.now() - 1;
    my_server->process_request(myclient_id, &request, &response);
    assert(response.accepted);
    assert(settings.max_local_requests == response.requests_remaining_local);
    assert(settings.max_remote_requests == response.requests_remaining_remote);

    cerr << "\nAfter processing a request for ALL: " << response << "\n" << *my_server;

    
    cerr << "\nenforcing timeouts";
    my_server->enforce_timeouts();
    
    cerr << "\nAfter processing a request for ALL:\n" << *my_server;
    return 0;
}


int main(int argc, char* argv[])
{
    cerr << "\nTesting init/cleanup routine";
    if (int init_ret = init(true))
    {
        cerr << "\ninit() got code " << init_ret << "; exiting\n\n";
        return init_ret;
    }
    cleanup();

    cerr << "\nTest wrapper checks out.";

    do_test("test0", test0);
    do_test("test_rb_all", test_rb_all);

    
    cerr << "\n\n";
    return 0;
    
}
