
#include "MCCIRevisionSet.h"
#include <stdio.h>

CMCCIRevisionSet::CMCCIRevisionSet(sqlite3* revision_db, string schema_signature)
{
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

    if ("" == currentsig)
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
    else
    {
        //FIXME: error or warning
    }

}



void CMCCIRevisionSet::load(sqlite3* revision_db)
{
    m_db = revision_db;
    sqlite3_exec(m_db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
    sqlite3_prepare_v2(m_db, "insert into revision(var_id, revision) values(?, 0)", 64, &m_insert, NULL);
    sqlite3_prepare_v2(m_db, "select revision from revision where var_id=?", 64, &m_read, NULL);
    sqlite3_prepare_v2(m_db, "update revision set revision=? where var_id=?", 64, &m_update, NULL);
}

void CMCCIRevisionSet::check_revision(int variableID)
{
    if (-1 == m_cache[variableID])
    {
        sqlite3_bind_int(m_read, 1, variableID);
        int result = sqlite3_step(m_read);
        
        switch (result)
        {
            case SQLITE_ROW:
                m_cache[variableID] = sqlite3_column_int(m_read, 0);
                break;
            case SQLITE_DONE:
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
        
        sqlite3_clear_bindings(m_read);
        sqlite3_reset(m_read);
    }
    
}

int CMCCIRevisionSet::get_revision(int variableID)
{
    check_revision(variableID);

    return m_cache[variableID];
}

int CMCCIRevisionSet::inc_revision(int variableID)
{
    check_revision(variableID);

    ++(m_cache[variableID]);

    sqlite3_bind_int(m_update, 1, variableID);
    sqlite3_bind_int(m_update, 2, m_cache[variableID]);
    sqlite3_step(m_update); //TODO: check return value
    sqlite3_clear_bindings(m_update);
    sqlite3_reset(m_update);

    return m_cache[variableID];

}
