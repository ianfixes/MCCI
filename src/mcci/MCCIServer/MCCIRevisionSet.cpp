
#include "MCCIRevisionSet.h"
#include <stdio.h>

CMCCIRevisionSet::CMCCIRevisionSet(sqlite3* revision_db, unsigned int schema_cardinality, string schema_signature)
{
    m_cache.resize_nearest_prime(schema_cardinality);
    load(revision_db);
    m_signature_id = 0;
    set_signature(schema_signature);
    m_strict = true;
}


CMCCIRevisionSet::CMCCIRevisionSet(const CMCCIRevisionSet &rhs)
{
    m_cache.resize(rhs.m_cache.get_size());
    load(rhs.m_db);
    m_strict = rhs.m_strict;
}


CMCCIRevisionSet::~CMCCIRevisionSet()
{
    sqlite3_finalize(m_insert);
    sqlite3_finalize(m_read);
    sqlite3_finalize(m_update);
}


ostream& operator<<(ostream& out, const CMCCIRevisionSet& rhs)
{
    out << "RevisionSet ("
        << rhs.get_signature()
        << "): ";

    for (LinearHash<MCCI_VARIABLE_T, MCCI_REVISION_T>::iterator it = rhs.m_cache.begin();
         it != rhs.m_cache.end(); ++it)
    {
        out << "\n\tVar # " << it->first << " : \t" << it->second;
    }

    
    return out;
}


string CMCCIRevisionSet::get_signature() const
{
    sqlite3_stmt* s;
    int result;
    string sig;

    sqlite3_prepare_v2(m_db, "select signature from signature where signature_id=?",
                       255, &s, NULL);
    sqlite3_bind_int(s, 1, m_signature_id);
    result = sqlite3_step(s);

    switch (result)
    {
        case SQLITE_ROW:
            sig = reinterpret_cast<const char*>(sqlite3_column_text(s, 0));
            break;
        case SQLITE_DONE:
            sig = "";
            break;
        default:
            char buffer[33];
            snprintf(buffer, 32, "Error in get_signature: %d", sqlite3_errcode(m_db));
            throw string(buffer);
    }
    
    sqlite3_finalize(s);

    return sig;
}


void CMCCIRevisionSet::set_signature(string signature)
{
    string currentsig;

    
    // early exits for errors or no-ops
    if (m_signature_id)
    {
        currentsig = get_signature();
        if (currentsig != signature && m_strict)
        {
            throw string("Tried to change signature from " + currentsig + " to " + signature);
        }

        if (currentsig == signature) return;
    }

    m_signature_id = lookup_signature_id(signature);

    if (m_signature_id) return; // if it exists, we're done

    //fprintf(stderr, "\ninserting '%s' signature", signature.c_str());
    sqlite3_stmt* s;
    sqlite3_prepare(m_db, "insert or ignore into signature(signature) values(?)",
                    255, &s, NULL);
    sqlite3_bind_text(s, 1, signature.c_str(), -1, SQLITE_TRANSIENT);
    int result = sqlite3_step(s);
    if (SQLITE_DONE != result)
    {
        string err = string("Error in set_signature insert: ") + string(sqlite3_errmsg(m_db));
        sqlite3_finalize(s);
        throw err;
    }
    sqlite3_finalize(s);

    m_signature_id = lookup_signature_id(signature);

}

int CMCCIRevisionSet::lookup_signature_id(string signature)
{
    int ret;
    sqlite3_stmt* s;

    sqlite3_prepare(m_db, "select signature_id from signature where signature=?",
                    255, &s, NULL);
    sqlite3_bind_text(s, 1, signature.c_str(), -1, SQLITE_TRANSIENT);
    int result = sqlite3_step(s);
    switch (result)
    {
        case SQLITE_ROW:  // already exists
            ret = sqlite3_column_int(s, 0); 
            break;
        case SQLITE_DONE:
            ret = 0;
            break;
        default:
            sqlite3_finalize(s);
            string err = string("Error in lookup_signature_id: ") + string(sqlite3_errmsg(m_db));
            throw err;
    }
    sqlite3_finalize(s);
    return ret;
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
    sqlite3_prepare_v2(m_db, "insert into revision(var_id, signature_id, revision) "
                       "values(?, ?, 0)",
                       255, &m_insert, NULL);
    sqlite3_prepare_v2(m_db, "select revision from revision where var_id=? and signature_id=?",
                       255, &m_read, NULL);
    sqlite3_prepare_v2(m_db, "update revision set revision=? where var_id=? and signature_id=?",
                       255, &m_update, NULL);


}

void CMCCIRevisionSet::check_revision(MCCI_VARIABLE_T variable_id)
{
    if (!m_cache.has_key(variable_id))
    {
        fprintf(stderr, "\ncheck_revision loading variable into cache");
        // bind placeholder #1 of the read statement to our new var id
        sqlite3_bind_int(m_read, 1, variable_id);
        sqlite3_bind_int(m_read, 2, m_signature_id);
        int result = sqlite3_step(m_read);  // look for the variable
        
        if (SQLITE_ROW == result)  // already exists
            m_cache[variable_id] = sqlite3_column_int(m_read, 0); 

        // record any error
        string err = string("Error in check_revision: ") + string(sqlite3_errmsg(m_db));
        // clean up from read
        sqlite3_clear_bindings(m_read);
        sqlite3_reset(m_read);

        if (SQLITE_ROW   == result) return;   // we retrieved the value and are done
        if (SQLITE_DONE != result) throw err; // "does not exist" is the only non-error case

        fprintf(stderr, "\ncheck_revision inserting variable into DB");
        // getting here means we need to INSERT a new record
        sqlite3_bind_int(m_insert, 1, variable_id);
        sqlite3_bind_int(m_insert, 2, m_signature_id);
        result = sqlite3_step(m_insert); 
        sqlite3_clear_bindings(m_insert);
        sqlite3_reset(m_insert);
        
        m_cache[variable_id] = 0; // matching the prepared statement

    }
    
}


MCCI_REVISION_T CMCCIRevisionSet::get_revision(MCCI_VARIABLE_T variable_id)
{
    check_revision(variable_id);

    return m_cache[variable_id];
}


MCCI_REVISION_T CMCCIRevisionSet::inc_revision(MCCI_VARIABLE_T variable_id)
{
    check_revision(variable_id);

    // immediate effect: memory
    ++(m_cache[variable_id]);

    // UPDATE existing revision.
    // scheduled effect: db (delayed write, not synchronous)
    sqlite3_bind_int(m_update, 1, m_cache[variable_id]);
    sqlite3_bind_int(m_update, 2, variable_id);
    sqlite3_bind_int(m_update, 3, m_signature_id);
    fprintf(stderr, "\ninc_revision updating variable #%ld's rev to %ld",
            (long)variable_id, (long)m_cache[variable_id]);
    int result = sqlite3_step(m_update); 
    switch (result)
    {
        case SQLITE_ROW:  
            throw string("inc_revision somehow got a row back from an update operation");
            break;
        case SQLITE_DONE: // this is the good case
            fprintf(stderr, "\ninc_revision OK");
            break;
        default:
            string err = string("Error in inc_revision: ") + string(sqlite3_errmsg(m_db));
            throw err;
    }

    sqlite3_clear_bindings(m_update);
    sqlite3_reset(m_update);

    return m_cache[variable_id];

}
