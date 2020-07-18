#include <iostream>
#include <ncurses.h>
#include <sqlite3.h>


struct sql_handle {
    sqlite3 *handle{nullptr};
};

void create_table(sql_handle &h, const std::string &table) {
    sqlite3_stmt *stmt;
    const char *unused;
    sqlite3_prepare_v2(h.handle, table.c_str(), table.size(), &stmt, &unused);
    while (sqlite3_step(stmt) != SQLITE_DONE) {}
    sqlite3_finalize(stmt);
}

void generate_schema(sql_handle &h) {
    // TODO: Check if db exists already.
    std::string version = "create table version (idx int primary key not null);";
    std::string language = "create table languages (idx int primary key not null,"
                           "deck_idx int, name text not null,"
                           "foreign key(deck_idx) references word(idx));";
    std::string words = "create table words (idx int primary key not null,"
                        "word text not null);";

    /*std::string folder =
        "create table folder (idx int primary key not null"
        "name text not null,"
        "parent_id int,"
        "foreign key(parent_id) references folder(id);";
    */
    create_table(h, version);
    create_table(h, words);
    create_table(h, language);
}

void init_sql(sql_handle &h) {
    std::cout << "Open\n";
    std::cout << sqlite3_open("database.db", &h.handle) << std::endl;
}

void close_sql(sql_handle &h) {
    std::cout << "closing sql" << std::endl;
    sqlite3_close(h.handle);
}


int main() {
    sql_handle handle;
    init_sql(handle);
    generate_schema(handle);
    close_sql(handle);
}
