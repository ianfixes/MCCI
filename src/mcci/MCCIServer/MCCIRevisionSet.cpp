
#include "MCCIRevisionSet.h"
#include <stdio.h>

CMCCIRevisionSet::CMCCIRevisionSet(sqlite3* revision_db, unsigned int schema_cardinality, string schema_signature)
{
    m_cache.resize_nearest_prime(schema_cardinality);
    load(revision_db);
    m_strict = true;
}


CMCCIRevisionSet::~CMCCIRevisionSet()
{
    sqlite3_finalize(m_insert);
    sqlite3_finalize(m_read);
    sqlite3_finalize(m_update);
}

string CMCCIRevisionSet::get_signature()
{
    sqlite3_stmt* s;
    int result;
    string sig;

    sqlite3_prepare_v2(m_db, "select signature from signature", 32, &s, NULL);
    result = sqlite3_step(s);

    switch (result)
    {
        case SQLITE_ROW:
            sig = reinterpret_cast<const char*>(sqlite3_column_text(s, 0));
            break;
        case SQLITE_DONE:
        default:
            //char buffer[33];
            //snprintf(buffer, 32, "%d", sqlite3_errcode(m_db));
            //sig = buffer;
            //TODO: something descriptive
            sig = "";
            break;
    }
    
    sqlite3_finalize(s);

    return sig;
}


void CMCCIRevisionSet::set_signature(string signature)
{
    string currentsig = get_signature();

    if ("" != currentsig)
    {
        throw string("Tried to change signature from " + currentsig + " to " + signature);
    }
    else
    {
        sqlite3_exec(m_db, "delete from signature", NULL, NULL, NULL); // FIXME: remove this? need way to reset
        
        fprintf(stderr, "\ninserting '%s' signature", signature.c_str());
        sqlite3_stmt* s;
        sqlite3_prepare(m_db, "insert into signature(signature) values(?)", 255, &s, NULL);
        sqlite3_bind_text(s, 1, signature.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(s);
        fprintf(stderr, "\nerrmsg is %s", sqlite3_errmsg(m_db));
        sqlite3_finalize(s);
    }
    

}



void CMCCIRevisionSet::load(sqlite3* revision_db)
{
    m_db = revision_db;

    // this optimization is OK because it only fails if the computer crashes
    // which is already a system failure requiring intervention
    sqlite3_exec(m_db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);

    // this optimization is NOT OK because it can corrupt the database if
    // the PROGRAM crashes.
    // sqlite3_exec(db, "PRAGMA journal_mode = MEMORY", NULL, NULL, NULL); // don't want this

    // prepare the statements that we will be using
    sqlite3_prepare_v2(m_db, "insert into revision(var_id, revision) values(?, 0)", 64, &m_insert, NULL);
    sqlite3_prepare_v2(m_db, "select revision from revision where var_id=?", 64, &m_read, NULL);
    sqlite3_prepare_v2(m_db, "update revision set revision=? where var_id=?", 64, &m_update, NULL);
}


void CMCCIRevisionSet::check_revision(MCCI_VARIABLE_T variableID)
{
    if (!m_cache.has_key(variableID))
    {
        // bind placeholder #1 of the read statement to our new var id
        sqlite3_bind_int(m_read, 1, variableID);
        int result = sqlite3_step(m_read);  // look for the variable
        
        switch (result)
        {
            case SQLITE_ROW:  // already exists
                m_cache[variableID] = sqlite3_column_int(m_read, 0); 
                break;
            case SQLITE_DONE: // does not exist
                sqlite3_bind_int(m_insert, 1, variableID);
                sqlite3_step(m_insert); //TODO: check return value
                sqlite3_clear_bindings(m_insert);
                sqlite3_reset(m_insert);
                
                m_cache[variableID] = 0; // matching the prepared statement
                break;
            default:
                //TODO: say what error we got?
                break;
        }

        // clean up
        sqlite3_clear_bindings(m_read);
        sqlite3_reset(m_read);
    }
    
}


MCCI_REVISION_T CMCCIRevisionSet::get_revision(MCCI_VARIABLE_T variableID)
{
    check_revision(variableID);

    return m_cache[variableID];
}


MCCI_REVISION_T CMCCIRevisionSet::inc_revision(MCCI_VARIABLE_T variableID)
{
    check_revision(variableID);

    // immediate effect: memory
    ++(m_cache[variableID]);

    // scheduled effect: db (delayed write, not synchronous)
    sqlite3_bind_int(m_update, 1, variableID);
    sqlite3_bind_int(m_update, 2, m_cache[variableID]);
    sqlite3_step(m_update); //TODO: check return value
    sqlite3_clear_bindings(m_update);
    sqlite3_reset(m_update);

    return m_cache[variableID];

}
