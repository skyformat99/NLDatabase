#pragma once


#include <string>
#include <sstream>
#include <sqlite3.h>



namespace NL {

namespace DB {
    

class Query {
public:
    ~Query() {
        sqlite3_finalize( stmt );
    }
    
    friend class Database;
    
private:
    sqlite3_stmt *stmt;
    
    Query( sqlite3_stmt *stmt ) : stmt( stmt ) {
    }
};


class Blob {
public:
    void *data;
    int length;
    
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
        return std::string( (char*)sqlite3_column_text( stmt, index ), sqlite3_column_bytes( stmt, index ) );
    }
    
    int column_int( int index ) const {
        return sqlite3_column_int( stmt, index );
    }
    
    double column_double( int index ) const {
        return sqlite3_column_double( stmt, index );
    }
    
    TransientBlob column_blob( int index ) const {
        return TransientBlob( sqlite3_column_blob( stmt, index ), sqlite3_column_bytes( stmt, index ) );
    }

    ~Row() {
        if ( finalize ) {
            sqlite3_finalize( stmt );
        }
    }
    
    friend class Cursor;
    friend class Database;
    
private:
    sqlite3_stmt* stmt;
    bool finalize;
    
    Row( sqlite3_stmt *stmt, bool finalize = false ) : stmt( stmt ), finalize( finalize ) {
    }
};


class Cursor {
public:
    bool operator!= ( const Cursor & it ) const {
        return is_valid != it.is_valid;
    }
    
    const Row & operator* () {
        return row;
    }
    
    Cursor & operator++ () {
        if ( sqlite3_step( stmt ) != SQLITE_ROW ) {
            is_valid = false;
        }
        return *this;
    }
    
private:
    Cursor( sqlite3_stmt *stmt, bool is_valid ) : stmt( stmt ), row( stmt ), is_valid( is_valid ) {
        if ( is_valid && sqlite3_step( stmt ) != SQLITE_ROW ) {
            is_valid = false;
        }
    }

    friend class Results;
    
    sqlite3_stmt *stmt;
    Row row;
    bool is_valid;
};


class Results {
public:
    ~Results() {
        if ( finalize ) {
            sqlite3_finalize( stmt );
        }
    }

    Cursor begin() const {
        return Cursor( stmt, true );
    }

    Cursor end() const {
        return Cursor( stmt, false );
    }
    
    typedef Cursor const_iterator;
    friend class Database;

private:
    sqlite3_stmt *stmt;
    bool finalize;
    
    Results( sqlite3_stmt *stmt, bool finalize ) : stmt( stmt), finalize( finalize ) {
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
    
    Query prepare( const std::string & query ) {
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
        query_single( "BEGIN" );
    }
    
    void commit() {
        query_single( "COMMIT" );
    }
    
    void rollback() {
        query_single( "ROLLBACK" );
    }
    
    int version() {
        return query_single( "PRAGMA user_version").column_int( 0 );
    }
    
    void set_version( const int version ) {
        std::ostringstream stream;
        stream << "PRAGMA user_version=" << version;
        query_single( stream.str() );
    }
    
    Results query( Query & query ) {
        sqlite3_reset( query.stmt );
        sqlite3_clear_bindings( query.stmt );
        return Results( query.stmt, false );
    }
    
    template <typename T, typename... Args>
    Results query( Query & query, T t, Args... args ) {
        sqlite3_reset( query.stmt );
        sqlite3_clear_bindings( query.stmt );
        set( query.stmt, 1, t, args... );
        return Results( query.stmt, false );
    }
    
    Results query( const std::string & query ) {
        sqlite3_stmt *stmt = 0;
        sqlite3_prepare_v2( db, query.c_str(), (int)query.length(), &stmt, 0 );
        return Results( stmt, true );
    }
    
    template <typename T, typename... Args>
    Results query( const std::string & query, T t, Args... args ) {
        sqlite3_stmt *stmt = 0;
        sqlite3_prepare_v2( db, query.c_str(), (int)query.length(), &stmt, 0 );
        set( stmt, 1, t, args... );
        return Results( stmt, true );
    }
    
    Row query_single( Query & query ) {
        sqlite3_reset( query.stmt );
        sqlite3_clear_bindings( query.stmt );
        sqlite3_step( query.stmt );
        return Row( query.stmt );
    }
    
    template <typename T, typename... Args>
    Row query_single( Query & query, T t, Args... args ) {
        sqlite3_reset( query.stmt );
        sqlite3_clear_bindings( query.stmt );
        set( query.stmt, 1, t, args... );
        sqlite3_step( query.stmt );
        return Row( query.stmt );
    }
    
    Row query_single( const std::string & query ) {
        sqlite3_stmt *stmt = 0;
        sqlite3_prepare_v2( db, query.c_str(), (int)query.length(), &stmt, 0 );
        sqlite3_step( stmt );
        return Row( stmt, true );
    }
    
    template <typename T, typename... Args>
    Row query_single( const std::string & query, T t, Args... args ) {
        sqlite3_stmt *stmt = 0;
        sqlite3_prepare_v2( db, query.c_str(), (int)query.length(), &stmt, 0 );
        set( stmt, 1, t, args... );
        sqlite3_step( stmt );
        return Row( stmt, true );
    }

    
private:
    sqlite3 *db;

    template <typename T>
    void set(sqlite3_stmt *stmt, int index, T value) {
        std::ostringstream stream;
        stream << value;
        std::string text( stream.str() );
        sqlite3_bind_text( stmt, index, text.c_str(), (int)text.length(), SQLITE_TRANSIENT );
    }
    
    template<typename T, typename... Args>
    void set(sqlite3_stmt *stmt, int index, T value, Args... args)
    {
        set( stmt, index, value );
        set( stmt, index+1, args...);
    }
};



template <>
void Database::set(sqlite3_stmt *stmt, int index, int value) {
    sqlite3_bind_int( stmt, index, value );
}

template <>
void Database::set(sqlite3_stmt *stmt, int index, double value) {
    sqlite3_bind_double( stmt, index, value );
}

template <>
void Database::set(sqlite3_stmt *stmt, int index, float value) {
    sqlite3_bind_double( stmt, index, (double) value );
}

template <>
void Database::set(sqlite3_stmt *stmt, int index, std::string value) {
    sqlite3_bind_text( stmt, index, value.c_str(), (int) value.length(), SQLITE_TRANSIENT );
}

template <>
void Database::set(sqlite3_stmt *stmt, int index, const char * value) {
    sqlite3_bind_text( stmt, index, value, (int) strlen( value ), SQLITE_TRANSIENT );
}

template <>
void Database::set(sqlite3_stmt *stmt, int index, char * value) {
    sqlite3_bind_text( stmt, index, value, (int) strlen( value ), SQLITE_TRANSIENT );
}

template <>
void Database::set(sqlite3_stmt *stmt, int index, StaticBlob value) {
    sqlite3_bind_blob( stmt, index, value.data, value.length, SQLITE_STATIC );
}

template <>
void Database::set(sqlite3_stmt *stmt, int index, TransientBlob value) {
    sqlite3_bind_blob( stmt, index, value.data, value.length, SQLITE_TRANSIENT );
}


} // namespace DB

} // namespace NL


