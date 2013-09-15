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
    
    for ( auto & row : results ) {
        cout << "column[0]=" << row.column_string( 0 ) << endl;
    }
    
    
    
    
    
    // Example 2: BLOBs of raw data, prepared statement, placing query call directly in for loop
    
    StaticBlob blob( "ABCDEF", 6 );
    
    Query query = db.prepare( "SELECT *, ? FROM test WHERE name=?" );
    
    for ( auto & row : db.query( query, blob, "TOM" ) ) {
        cout << "column[0]=" << row.column_string( 0 ) << endl;
        cout << "column[2]=" << row.column_string( 2 ) << endl;
    }

    
    
    
    
    // Example 3: Open database, make a query and fetch results in a single line.
    
    for ( auto & row : Database( path ).query( "SELECT * FROM test ORDER BY ?", "name")) {
        cout << "column[0]=" << row.column_string( 0 ) << endl;
    }
    
    return 0;
}
