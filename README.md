# NLDatabase

Lightweight C++ wrapper for SQLite.

## Requirements

 - C++11 compiler
 - SQLite 3

## Usage

Let's open a database file and read some rows:

    #include "NLDatabase.h"


	using namespace std;
	using namespace NL::DB;


	int main(int argc, const char * argv[]) {

	    Database db( "test.sqlite" );
	    
	    auto results = db.query( "SELECT * FROM test WHERE name <> ?" ).select( "TOM" );
	    
	    for ( auto & row : results ) {
	        cout << "column[0]=" << row.column_string( 0 ) << endl;
	    }
	}

Check out the demo for more examples.

## Installation

It's a single header, just drop into your project, link sqlite3 and that's it.

## Demo

To run the simple demo, use the Xcode project or build it on the command line like this:

	sqlite3 test.sqlite < test.sql
	clang -o test -std=c++11 -lsqlite3 -lstdc++ main.cpp
	./test


## Known problems

 - There is no error checking. None. Your queries must be perfect. Also, patches are welcome.
 - If you look at the way the results are retrieved, you will realize that you can only iterate through them ONCE. I didn't see an obvious way to express this in code and make it impossible syntactically, so it's simply something you need to keep in mind.

## Personal note

If you use NLDatabase in a project, I'd love to hear about it. Please do let me know at tomovo@gmail.com. Thanks!

## See also

Goes well with [NLTemplate](https://github.com/catnapgames/NLTemplate) and possibly [Mongoose](https://github.com/cesanta/mongoose).

