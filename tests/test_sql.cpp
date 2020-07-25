#include <iostream>
#include <ncurses.h>
#include <sql.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <cassert>

bool cmp_str(const std::string &s1, const std::string &s2) {
    return s1 == s2;
}

int callback(void *data, int ncols, char **col_values, char **col_names) {
    std::cout << std::endl;
    std::cout << "Callback " << std::endl;
    for (int i = 0; i < ncols; i++) {
        std::cout << col_names[i] << ": " << col_values[i] << "\n";
        if (cmp_str(col_names[i], "idx")) {
            *(int *)data = atoi(col_values[i]);
        }
    }

    return 0;
}

int main() {
    sql_db db;
    db.name = "test_db.db";
    open_db(db);

    if (!database_already_exists(db)) {
        generate_schema(db);
        insert_into(db, "version", "idx", 1);
    }

    insert_into(db, "languages", "idx", 1, "name", "Russian");

    int lang_idx = 1337;
    select_from(db, "select * from languages where name='Russian';", callback, &lang_idx);
    assert(lang_idx == 1);
    insert_into(db, "decks", "lang_idx", lang_idx, "name", "Chapter 1");

    int deck_idx = 4444;
    select_from(db, "select * from decks where name='Chapter 1';", callback, &deck_idx);
    assert(deck_idx == 1);

    int word_idx = 9999;
    insert_into(db, "words", "word", "здравствуйте");
    select_from(db, "select * from words;", callback, &word_idx);
    assert(word_idx == 1);

    insert_into(db, "deck_word", "word_idx", word_idx, "deck_idx", deck_idx);

    close_db(db);

    return 0;
}
