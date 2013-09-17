#include <iostream>
#include <string>

#include "NLDatabase.h"


using namespace std;
using namespace NL::DB;


static const char *path = "test.sqlite";


int main(int argc, const char * argv[]) {

    Database db( path );
    
    
    
    
    // Example 1: simple query with two parameters.
    
    auto results = db.query("SELECT * FROM test WHERE name <> ? AND name <> ?", string( "GEORGE" ), "TOM" );
    
    for ( auto const & row : results ) {
        cout << "COLUMN[0]=" << row.column_string( 0 ) << endl;
    }
    
    
    
    
    // Example 2: BLOBs of raw data, prepared statement, placing query call directly in for loop
    
    StaticBlob blob( "ABCDEF", 6 );
    
    Query query = db.prepare( "SELECT *, ? FROM test WHERE name=?" );
    
    for ( auto const & row : db.query( query, blob, "TOM" ) ) {
        cout << "COLUMN[0]=" << row.column_string( 0 ) << endl;
        cout << "COLUMN[2]=" << row.column_string( 2 ) << endl;
    }
    
    
    
    
    // Example 3: Open database, make a query and fetch results in a single line.
    
    for ( auto const & row : Database( path ).query( "SELECT * FROM test ORDER BY ?", "name")) {
        cout << "COLUMN[0]=" << row.column_string( 0 ) << endl;
    }
    
    
    
    
    // Example 4: Fetch a single row with a result and access it directly without iterating
    
    int count = db.query_single( "SELECT COUNT(1) FROM test").column_int( 0 );
    cout << "COUNT=" << count << endl;
    
    
    
    
    
    // Example 5: Simple support for versioning. Could allow a simple migration scheme.
    
    int version = db.version();
    cout << "VERSION=" << version << endl;
    
    db.set_version( version + 1 );
    cout << "VERSION=" << db.version() << endl;
    
    
    
    
    
    // Example 5: Transactions, getting number of affected rows
    
    db.begin();
    
    db.query_single( "UPDATE test SET name=? WHERE name=?", "PAUL", "GEORGE");
    
    int num_rows_affected = db.changes();

    cout << "AFFECTED " << db.changes() << " ROWS" << endl;

    if ( num_rows_affected == 0 ) {
        db.query_single( "UPDATE test SET name=? WHERE name=?", "GEORGE", "PAUL"); // swap them back
    }

    db.commit();
    
    for ( auto const & row : Database( path ).query( "SELECT * FROM test ORDER BY ?", "name")) {
        cout << "COLUMN[0]=" << row.column_string( 0 ) << endl;
    }

    
    
    
    
    // Example 5: Get last insert row id
    
    db.query_single( "INSERT INTO test (name) VALUES(?)", "FRANK" );
    
    auto id = db.last_insert_rowid();
    
    cout << "FRANK ID=" << id << endl;
    
    db.query_single( "DELETE FROM test WHERE name=?", "FRANK" );

    
    return 0;
}
