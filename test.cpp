#include <iostream>
#include <algorithm>
#include <string>

#include "NLDatabase.h"


using namespace std;
using namespace NL::DB;


static const string path = "test.sqlite";
static const char tab = '\t';


void migrate( Database & db ) {
    db.begin();

    // fallthrough to do all necessary steps of migration
    switch ( db.version() + 1 ) {
        case 1:
            db.query( "CREATE TABLE test (name TEXT)" ).execute();
            db.query( "INSERT INTO test VALUES(?)" ).execute("TOM");
            db.query( "INSERT INTO test VALUES(?)" ).execute("JACK");
        case 2:
            db.query( "ALTER TABLE test ADD COLUMN country TEXT").execute();
            db.query( "UPDATE test SET country=NULL").execute();
        case 3:
            db.query( "INSERT INTO test VALUES(?, NULL)" ).execute("GEORGE");
            db.query( "INSERT INTO test VALUES(?, NULL)" ).execute("JOHN");
    }
    
    db.set_version( 3 ); // set to latest version
    db.commit();
}


int find_column_index( const string & name, const vector<string> & names ) {
    for ( int i=0; i < names.size(); i++ ) {
        if ( names[ i ] == name ) {
            return i;
        }
    }
    
    return -1;
}


int main(int argc, const char * argv[]) {

    Database db( path );

    
    
    cout << "Example 0: simple way to do schema version migrations" << endl;

    migrate( db );
    
    
    
    cout << "Example 1: simple query with two parameters" << endl;
    
    auto results = db.query("SELECT * FROM test WHERE name <> ? AND name <> ?").select( "GEORGE", "TOM" );
    
    for ( auto const & row : results ) {
        cout << tab << "COLUMN[0] = " << row.column_string( 0 ) << endl;
    }
    
    
    
    
    cout << "Example 2: BLOBs of raw data, select call directly in for loop" << endl;
    
    StaticBlob blob( "ABCDEF", 6 );
    
    Query query = db.query( "SELECT *, ? FROM test WHERE name=?" );
    
    for ( auto const & row : query.select( blob, "TOM" ) ) {
        cout << tab << "COLUMN[0] = " << row.column_string( 0 ) << endl;
        cout << tab << "COLUMN[2] = " << row.column_string( 2 ) << endl;
    }
    
    
    
    
    cout << "Example 3: Open database, make a query and fetch results in a single line" << endl;
    
    for ( auto const & row : Database( path ).query( "SELECT * FROM test ORDER BY ?" ).select("name")) {
        cout << tab << "COLUMN[0] = " << row.column_string( 0 ) << endl;
    }
    
    
    
    
    cout << "Example 4: Fetch a single row with a result and access it directly without iterating" << endl;
    
    int count = db.query( "SELECT COUNT(1) FROM test").select().single().column_int( 0 );
    cout << tab << "COUNT = " << count << endl;
    
    

    
    
    cout << "Example 5: Simple support for versioning, which is also used by our simple migration scheme" << endl;
    
    int version = db.version();
    cout << tab << "VERSION = " << version << endl;
    
    db.set_version( version + 1 );
    cout << tab << "VERSION = " << db.version() << endl;
    
    
    
    
    
    cout << "Example 6: Transactions, getting number of affected rows" << endl;
    
    db.begin();
    
    db.query( "UPDATE test SET name=? WHERE name=?").execute("PAUL", "GEORGE");
    
    int num_rows_affected = db.changes();

    cout << tab << "AFFECTED " << num_rows_affected << " ROWS" << endl;

    if ( num_rows_affected == 0 ) {
        db.query( "UPDATE test SET name=? WHERE name=?").execute("GEORGE", "PAUL"); // swap them back
    }

    db.commit();
    
    for ( auto const & row : Database( path ).query( "SELECT * FROM test ORDER BY ?").select("name")) {
        cout << tab << "COLUMN AFTER COMMIT[0] =" << row.column_string( 0 ) << endl;
    }

    db.begin();
    
    db.query( "UPDATE test SET name=?").execute("JIM");
    
    db.rollback();

    for ( auto const & row : Database( path ).query( "SELECT * FROM test ORDER BY ?").select("name")) {
        cout << tab << "COLUMN AFTER ROLLBACK[0] = " << row.column_string( 0 ) << endl;
    }

    
    
    
    
    cout << "Example 7: Get last insert row id" << endl;
    
    db.query( "INSERT INTO test (name) VALUES(?)").execute("FRANK");

    auto id = db.last_insert_rowid();

    cout << tab << "FRANK ID=" << id << endl;

    db.query( "DELETE FROM test WHERE name=?").execute("FRANK" );




    
    cout << "Example 8: Get column names" << endl;

    auto query2 = db.query( "SELECT *, 1, 2, 3, 4 AS 'custom' FROM test");
    auto names = query2.column_names();
    
    for ( auto const & column_name : names ) {
        cout << tab << "Column: " << column_name << endl;
    }

    // SQLite doesn't have a "get column by name" function, you have to get all names and then find what you need.
    
    int name_index = find_column_index( "name", names );
    
    for ( auto const & row : query2.select() ) {
        cout << tab << "name = " << row.column_string( name_index ) << endl;
    }
    
    return 0;
}
