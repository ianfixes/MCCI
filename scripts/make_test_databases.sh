#!/usr/bin/env sh
echo "Creating databases that are used by MCCI test programs."

sqlite3 db.sqlite3 < src/sql/example-mcci-schema.sql
sqlite3 revisions.sqlite3 < src/sql/revisions.sql
