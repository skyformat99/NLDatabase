#include "NLDatabase.h"


namespace NL {
    
namespace DB {
        

template <>
void Query::set(sqlite3_stmt *stmt, int index, int value) {
    sqlite3_bind_int( stmt, index, value );
}

template <>
void Query::set(sqlite3_stmt *stmt, int index, double value) {
    sqlite3_bind_double( stmt, index, value );
}

template <>
void Query::set(sqlite3_stmt *stmt, int index, float value) {
    sqlite3_bind_double( stmt, index, (double) value );
}

template <>
void Query::set(sqlite3_stmt *stmt, int index, std::string value) {
    sqlite3_bind_text( stmt, index, value.c_str(), (int) value.length(), SQLITE_TRANSIENT );
}

template <>
void Query::set(sqlite3_stmt *stmt, int index, const char * value) {
    sqlite3_bind_text( stmt, index, value, (int) strlen( value ), SQLITE_TRANSIENT );
}

template <>
void Query::set(sqlite3_stmt *stmt, int index, char * value) {
    sqlite3_bind_text( stmt, index, value, (int) strlen( value ), SQLITE_TRANSIENT );
}

template <>
void Query::set(sqlite3_stmt *stmt, int index, StaticBlob value) {
    sqlite3_bind_blob( stmt, index, value.data, value.length, SQLITE_STATIC );
}

template <>
void Query::set(sqlite3_stmt *stmt, int index, TransientBlob value) {
    sqlite3_bind_blob( stmt, index, value.data, value.length, SQLITE_TRANSIENT );
}

    
} // namespace DB
    
} // namespace NL

