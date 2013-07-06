
#include "MCCIRevisionSet.h"

#include <string.h>
#include <sqlite3.h>
#include <iostream>
#include <assert.h>

using namespace std;

sqlite3* rs_db     = NULL;

CMCCIRevisionSet* rs = NULL;


void cleanup()
{
    delete rs;
    
    sqlite3_close(rs_db);
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
    
    if (!try_open_db(debug, "../../../revisions.sqlite3", &rs_db, SQLITE_OPEN_READWRITE))
    {
        cleanup();
        return 1;
    }

    try
    {
        if (debug) cerr << "\nCreating revisionset object...";
        rs     = new CMCCIRevisionSet(rs_db, 5, "test_signature");
        if (debug)
        {
            cerr << "OK";
            cerr << "\n -- " << *rs;
        }
        
    }
    catch (std::bad_alloc ba)
    {
        cerr << "\nGot exception '" <<  ba.what() << "'";
        return 1;
    }
    catch (string s)
    {
        cerr << "\n\nGot error: " << s << "\n\n";
        return 1;
    }
    catch (...)
    {
        cerr << "\n well... we caught some error";
        return 1;
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

// request is a request packet, impacts are how many req slots we expect the packet to occupy
int test_incrementing()
{
    MCCI_REVISION_T this_rev = rs->get_revision(1);
    cerr << "\nthis_rev = " << this_rev;
    
    MCCI_REVISION_T next_rev = rs->inc_revision(1);
    cerr << "\nnext_rev = " << next_rev;
    
    assert(this_rev + 1 == next_rev);

    cerr << "\nget_revision(1) == " << rs->get_revision(1);
    assert(next_rev == rs->get_revision(1));

    cerr << "\nReloading RevisionSet to verify persistence";
    delete rs;
    rs = new CMCCIRevisionSet(rs_db, 5, "test_signature");

    cerr << "\nget_revision(1) == " << rs->get_revision(1);
    assert(next_rev == rs->get_revision(1));
    
    return 0;
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


    do_test("test_incrementing", test_incrementing);

    cleanup();
    cerr << "\n\n";
    return 0;
    
}
