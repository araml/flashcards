#include <sql.h>
#include <iostream>

sql_db d;

auto empty_callback = [] (void *, int, char **, char **) { return 0; };

int create_table(sql_db &d, const std::string &table) {
    char *error;
    if (sqlite3_exec(d.db, table.c_str(), empty_callback, nullptr, &error)
        != SQLITE_OK) {
        return 1;
    }
    return 0;
}

int generate_schema(sql_db &d) {
    std::string version = "create table version (idx integer primary key not null);";
    std::string language = "create table languages (idx integer primary key not null,"
                           "name text not null);";
    std::string decks = "create table decks (idx integer primary key not null,"
                        "lang_idx int, name text not null,"
                        "foreign key(lang_idx) references languages(idx));";

    std::string words = "create table words (idx integer primary key not null,"
                                            "word text not null);";

    std::string deck_word = "create table deck_word (word_idx int, deck_idx int,"
                            "foreign key(word_idx) references words(idx),"
                            "foreign key(deck_idx) references decks(idx));";


    int return_value = 0;

    return_value += create_table(d, version);
    return_value += create_table(d, language);
    return_value += create_table(d, decks);
    return_value += create_table(d, words);
    return_value += create_table(d, deck_word);

    return return_value;
}




/* We try to query the db, if it succeeds we must have already created the
 * schema.
 */
bool database_already_exists(sql_db &d) {
    char *err;
    const char select_version[] = "select * from version;";
    if (sqlite3_exec(d.db, select_version,
                 [](void *, int, char **, char **) -> int { return 0; },
                 nullptr, &err) == SQLITE_OK) {
        return true;
    }

    return false;
}

void init_sql(sql_db &d) {
    std::cout << "Open\n";
    std::cout << sqlite3_open("database.db", &d.db) << std::endl;
}

void close_sql(sql_db &d) {
    std::cout << "closing sql" << std::endl;
    sqlite3_close(d.db);
}

