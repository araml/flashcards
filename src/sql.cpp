#include <sql.h>
#include <iostream>

sql_db d;

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

    std::string languages =
        "create table languages ("
        "idx integer not null primary key,"
        "name text not null unique);";

    std::string decks = "create table decks ("
                            "idx integer not null primary key,"
                            "lang_idx integer,"
                            "name text not null,"
                            "foreign key(lang_idx) references languages(idx));";

    // TODO: we won't handle the time for now
    std::string words = "create table words (idx integer primary key not null,"
                                            "word text not null unique,"
                                            "time text);";

    std::string deck_word = "create table deck_word ("
                                "word_idx integer,"
                                "deck_idx integer,"
                                "foreign key(word_idx) references words(idx),"
                                "foreign key(deck_idx) references decks(idx));";


    int return_value = 0;

    return_value += create_table(d, version);
    return_value += create_table(d, languages);
    return_value += create_table(d, decks);
    return_value += create_table(d, words);
    return_value += create_table(d, deck_word);

    return return_value;
}

/* We try to query the db, if it succeeds we must have already created the
 * schema.
 */
int database_already_exists(sql_db &d) {
    //const char select_version[] = "select * from version;";
    if (select_from(d, "select * from version;")) {
        return 1;
    }

    return 0;
}

void open_db(sql_db &d) {
    std::cout << "Open\n";
    std::cout << sqlite3_open(d.name.c_str(), &d.db) << std::endl;
}

void close_db(sql_db &d) {
    std::cout << "closing sql" << std::endl;
    sqlite3_close(d.db);
}

