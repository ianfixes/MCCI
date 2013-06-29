
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
CMCCIServerNetworkingFake fake_networking(cerr);

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
        
        settings.my_node_address = 5;
        settings.max_local_requests = 101;
        settings.max_remote_requests = 199;
        settings.max_clients = 100;

        // these numbers will automatically be adjusted to prime numbers for the hash tables
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
        my_server = new CMCCIServer((CMCCITime*)&fake_time,
                                    (CMCCIServerNetworking*)&fake_networking,
                                    settings);
        if (debug) cerr << "OK";
        if (debug) cerr << "\n" << my_server->get_settings();

        if (debug) cerr << "\nProducing values";
        for (int i = 0; i < 60; ++i)
        {
            SMCCIProductionPacket p;
            p.variable_id = 2;
            p.response_id = 0;

            SMCCIAcceptancePacket a;
            
            my_server->process_production(36, &p, &a);

            if (debug) cerr << ".";
        }
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
    bool no_error = false;
    
    if (int init_ret = init(false))
    {
        cerr << "\ninit() got code " << init_ret << "; exiting\n\n";
        cleanup();
        return init_ret;
    }

    cerr << "\n\n---------------- LET THE TESTING BEGIN: " << name << "\n";
    try
    {
        if (int test_ret = test_fn())
        {
            cerr << "\ntest_fn '" << name << "' got code " << test_ret << "; exiting\n\n";
            cleanup();
            assert(0 == test_ret);
        }
    }
    catch (string s)
    {
        cerr << "\n\nTest " << name << " got error: " << s << "\n\n";
        assert(no_error);
    }
    catch (...)
    {
        cerr << "\n Test " << name << " caught some error";
        assert(no_error);
    }

    cerr << "\n -- Test " << name << " done ";
    cleanup();
    cerr << "and cleaned up";

    return 0;
    
}

int test0()
{
    cerr << "\nEmpty server\n" << *my_server;
    return 0;
}

// request is a request packet, impacts are how many req slots we expect the packet to occupy
int test_rb_basic(SMCCIRequestPacket request, int local_impact, int remote_impact, int reqs)
{
    MCCI_CLIENT_ID_T myclient_id = 37;
    SMCCIServerSettings settings = my_server->get_settings();
    fake_time.set_now(12344);

    request.timeout = fake_time.now();
    
    SMCCIResponsePacket response;
    response.accepted = false;
    response.requests_remaining_local = 0;
    response.requests_remaining_remote = 0;

    cerr << "\nTesting request acceptance: ";
    request.timeout = fake_time.now() + 1;
    my_server->process_request(myclient_id, &request, &response);
    cerr << response;
    assert(response.accepted);
    assert(settings.max_local_requests == response.requests_remaining_local + local_impact);
    assert(settings.max_remote_requests == response.requests_remaining_remote + remote_impact);

    cerr << "\nreqs " << reqs << " rc " << my_server->request_count();
    assert(reqs == my_server->request_count());

    cerr << "\nenforcing timeouts after a request w/ future timeout: " << response;
    my_server->enforce_timeouts();
    assert(reqs == my_server->request_count());
    cerr << "\n" << *my_server;
    
    cerr << "\nTesting request alteration: ";
    request.timeout = fake_time.now() - 1;
    my_server->process_request(myclient_id, &request, &response);
    cerr << response;
    assert(response.accepted);
    assert(settings.max_local_requests == response.requests_remaining_local + local_impact);
    assert(settings.max_remote_requests == response.requests_remaining_remote + remote_impact);

    cerr << "\nAfter altering a request to a past timeout: " << response << "\n" << *my_server;

    
    cerr << "\nenforcing timeouts";
    my_server->enforce_timeouts();
    assert(0 == my_server->request_count());
    
    cerr << "\nAfter enforcing timeouts:\n" << *my_server;
    return 0;
}

int test_rb_all()
{    
    SMCCIRequestPacket request;
    request.node_address = MCCI_HOST_ANY;
    request.variable_id = 0;
    request.revision = 0;
    request.quantity = 1;

    // requests for ALL don't count against totals!
    return test_rb_basic(request, 0, 0, 1);
}

int test_rb_host()
{    
    SMCCIRequestPacket request;
    request.node_address = 88;
    request.variable_id = 0;
    request.revision = 0;
    request.quantity = 1;

    // host requests count against the remote
    return test_rb_basic(request, 0, 1, 1);
}

int test_rb_host_loc()
{    
    SMCCIRequestPacket request;
    request.node_address = 0;
    request.variable_id = 0;
    request.revision = 0;
    request.quantity = 1;

    // host requests count against remote
    return test_rb_basic(request, 0, 1, 1);
}

int test_rb_var()
{    
    SMCCIRequestPacket request;
    request.node_address = MCCI_HOST_ANY;
    request.variable_id = 1;
    request.revision = 0;
    request.quantity = 1;

    // variable requests count against remote
    return test_rb_basic(request, 0, 1, 1);
}

int test_rb_hostvar()
{    
    SMCCIRequestPacket request;
    request.node_address = 88;
    request.variable_id = 1;
    request.revision = 0;
    request.quantity = 1;

    // host/variable requests count against remote
    return test_rb_basic(request, 0, 1, 1);
}

int test_rb_remote()
{    
    SMCCIRequestPacket request;
    request.node_address = 88;
    request.variable_id = 1;
    request.revision = 61;
    request.quantity = 5;

    // remote requests count against remote
    return test_rb_basic(request, 0, 5, 1);
}

int test_rb_varrev()
{
    SMCCIRequestPacket request;
    request.node_address = 0;
    request.variable_id = 1;
    request.revision = 61;
    request.quantity = 5;

    // varrev requests count against local
    return test_rb_basic(request, 5, 0, 1);
}




int main(int argc, char* argv[])
{
    cerr << "\n================= Testing init/cleanup routine =================";
    cerr << "==================================================================";
    if (int init_ret = init(true))
    {
        cerr << "\ninit() got code " << init_ret << "; exiting\n\n";
        return init_ret;
    }
    cleanup();

    cerr << "\nTest wrapper checks out.";

    do_test("test0", test0);
    do_test("test_rb_all", test_rb_all);
    do_test("test_rb_host", test_rb_host);
    do_test("test_rb_host_loc", test_rb_host_loc);
    do_test("test_rb_var", test_rb_var);
    do_test("test_rb_hostvar", test_rb_hostvar);
    do_test("test_rb_remote", test_rb_remote);
    do_test("test_rb_varrev", test_rb_varrev);

    cerr << "\n\n";
    return 0;
    
}
