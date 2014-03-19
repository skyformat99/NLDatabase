#pragma once


#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <sqlite3.h>



namespace NL {

namespace DB {
    
    
class Blob {
public:
    void *data;
    int length;

protected:
    Blob( void *data, int length ) : data( data ), length( length ) {
    }
};


class StaticBlob : public Blob {
public:
    template<typename T>
    StaticBlob( T *data, int length ) : Blob( (void*) data, length ) { }
};

    
class TransientBlob : public Blob {
public:
    template<typename T>
    TransientBlob( T *data, int length ) : Blob( (void*) data, length ) { }
};


class Row {
public:
    std::string column_string( int index ) const {
        return std::string( (char*)sqlite3_column_text( stmt.get(), index ), sqlite3_column_bytes( stmt.get(), index ) );
    }
    
    int column_int( int index ) const {
        return sqlite3_column_int( stmt.get(), index );
    }
    
    double column_double( int index ) const {
        return sqlite3_column_double( stmt.get(), index );
    }
    
    TransientBlob column_blob( int index ) const {
        return TransientBlob( sqlite3_column_blob( stmt.get(), index ), sqlite3_column_bytes( stmt.get(), index ) );
    }

    friend class Cursor;
    friend class Query;
    friend class Results;
    
private:
    std::shared_ptr<struct sqlite3_stmt> stmt;
    
    Row( const std::shared_ptr<struct sqlite3_stmt> & stmt ) : stmt( stmt ) {
    }
};


class Cursor {
public:
    bool operator!= ( const Cursor & it ) const {
        return is_valid != it.is_valid;
    }
    
    const Row & operator* () const {
        return row;
    }
    
    Cursor & operator++ () {
        if ( sqlite3_step( stmt.get() ) != SQLITE_ROW ) {
            is_valid = false;
        }
        return *this;
    }
    
private:
    Cursor( const std::shared_ptr<struct sqlite3_stmt> & stmt, bool is_valid ) : stmt( stmt ), row( stmt ), is_valid( is_valid ) {
        if ( is_valid && sqlite3_step( stmt.get() ) != SQLITE_ROW ) {
            this->is_valid = false;
        }
    }

    friend class Results;
    
    std::shared_ptr<struct sqlite3_stmt> stmt;
    Row row;
    bool is_valid;
};


class Results {
public:
    const Cursor begin() const {
        return Cursor( stmt, true );
    }

    const Cursor end() const {
        return Cursor( stmt, false );
    }
    
    Row single() const {
        sqlite3_step( stmt.get() );
        return Row( stmt );
    }
    
    typedef Cursor const_iterator;
    friend class Query;

private:
    std::shared_ptr<struct sqlite3_stmt> stmt;
    
    Results( const std::shared_ptr<struct sqlite3_stmt> & stmt ) : stmt( stmt) {
    }
};


class Query {
public:
    Results select() {
        sqlite3_reset( stmt.get() );
        sqlite3_clear_bindings( stmt.get() );
        return Results( stmt );
    }
    
    template <typename T, typename... Args>
    Results select( T t, Args... args ) {
        sqlite3_reset( stmt.get() );
        sqlite3_clear_bindings( stmt.get() );
        set( stmt.get(), 1, t, args... );
        return Results( stmt );
    }
    
    void execute() {
        sqlite3_step( stmt.get() );
    }
    
    template <typename T, typename... Args>
    void execute( T t, Args... args ) {
        sqlite3_reset( stmt.get() );
        sqlite3_clear_bindings( stmt.get() );
        set( stmt.get(), 1, t, args... );
        sqlite3_step( stmt.get() );
    }
    
    std::vector<std::string> column_names() const {
        int count = sqlite3_column_count( stmt.get() );
        std::vector<std::string> names;
        for ( int i=0; i < count; i++ ) {
            names.emplace_back( sqlite3_column_name( stmt.get(), i ) );
        }
        return names;
    }
    
    friend class Database;
    
private:
    std::shared_ptr<struct sqlite3_stmt> stmt;
    
    template <typename T>
    void set( sqlite3_stmt *stmt, int index, T value) {
        std::ostringstream stream;
        stream << value;
        std::string text( stream.str() );
        sqlite3_bind_text( stmt, index, text.c_str(), (int)text.length(), SQLITE_TRANSIENT );
    }
    
    template<typename T, typename... Args>
    void set( sqlite3_stmt *stmt, int index, T value, Args... args) {
        set( stmt, index, value );
        set( stmt, index+1, args...);
    }
    
    Query( sqlite3_stmt *stmt ) : stmt( stmt, sqlite3_finalize ) {
    }
};

    
class Database {
public:
    Database( const std::string & path ) {
        sqlite3_open( path.c_str(), &db );
    }
    
    ~Database() {
        sqlite3_close( db );
    }
    
    Query query( const std::string & query ) {
        sqlite3_stmt *stmt = 0;
        sqlite3_prepare_v2( db, query.c_str(), (int)query.length(), &stmt, 0 );
        return Query( stmt );
    }
    
    int changes() const {
        return sqlite3_changes( db );
    }
    
    long long last_insert_rowid() const {
        return sqlite3_last_insert_rowid( db );
    }
    
    void begin() {
        query( "BEGIN").execute();
    }
    
    void commit() {
        query( "COMMIT" ).execute();
    }
    
    void rollback() {
        query( "ROLLBACK" ).execute();
    }
    
    int version() {
        return query( "PRAGMA user_version").select().single().column_int( 0 );
    }
    
    void set_version( const int version ) {
        std::ostringstream stream;
        stream << "PRAGMA user_version=" << version;
        query( stream.str() ).execute();
    }
    
private:
    sqlite3 *db;
};

    
    template <> void Query::set(sqlite3_stmt *stmt, int index, int value);
    template <> void Query::set(sqlite3_stmt *stmt, int index, double value);
    template <> void Query::set(sqlite3_stmt *stmt, int index, float value);
    template <> void Query::set(sqlite3_stmt *stmt, int index, std::string value);
    template <> void Query::set(sqlite3_stmt *stmt, int index, const char * value);
    template <> void Query::set(sqlite3_stmt *stmt, int index, char * value);
    template <> void Query::set(sqlite3_stmt *stmt, int index, StaticBlob value);
    template <> void Query::set(sqlite3_stmt *stmt, int index, TransientBlob value);

} // namespace DB

} // namespace NL


