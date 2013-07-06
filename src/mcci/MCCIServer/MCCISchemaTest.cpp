

#include "MCCISchema.h"

#include <string.h>
#include <sqlite3.h>
#include <stdio.h>
#include <assert.h>

using namespace std;

sqlite3* schema_db = NULL;


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

    unsigned char input[32] = "abcde\0";
    char output[64];
    
    CMCCISchema::b64_encode(input, output, 6, 64);

    printf("\n\nB64 test: input='%s', output='%s'\n", input, output);
    assert(string("YWJjZGUA") == string(output));
    
    CMCCISchema* schema = NULL;
    
    printf("\nOpening database...");
    if (!try_open_db("../../../db.sqlite3", &schema_db, SQLITE_OPEN_READONLY))
    {
        sqlite3_close(schema_db);
        printf("FAIL");
        return 1;
    }
    printf("OK");

    printf("\nInstantiating schema obj...");
    schema = new CMCCISchema(schema_db);
    printf("OK");

    printf("\nCardinality is %d", schema->get_cardinality());

    printf("\nSchema:");
    for (int i = 0; i < schema->get_cardinality(); ++i)
    {
        MCCI_VARIABLE_T v = schema->variable_of_ordinal(i);
        printf("\n   %d\t%d\t%s", i, v, schema->name_of_variable(v).c_str());
    }

    printf("\nHash: %s", schema->get_hash().c_str());

    delete schema;
    sqlite3_close(schema_db);
    
    printf("\n\n");

    return 0;
}
