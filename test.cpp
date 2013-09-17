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
        cout << "column[0]=" << row.column_string( 0 ) << endl;
    }
    
    
    
    
    // Example 2: BLOBs of raw data, prepared statement, placing query call directly in for loop
    
    StaticBlob blob( "ABCDEF", 6 );
    
    Query query = db.prepare( "SELECT *, ? FROM test WHERE name=?" );
    
    for ( auto const & row : db.query( query, blob, "TOM" ) ) {
        cout << "column[0]=" << row.column_string( 0 ) << endl;
        cout << "column[2]=" << row.column_string( 2 ) << endl;
    }
    
    
    
    
    // Example 3: Open database, make a query and fetch results in a single line.
    
    for ( auto const & row : Database( path ).query( "SELECT * FROM test ORDER BY ?", "name")) {
        cout << "column[0]=" << row.column_string( 0 ) << endl;
    }
    
    
    
    
    // Example 4: Fetch a single row with a result and access it directly without iterating
    
    int count = db.query_row( "SELECT COUNT(1) FROM test").column_int( 0 );
    cout << "COUNT = " << count << endl;
    
    return 0;
}
