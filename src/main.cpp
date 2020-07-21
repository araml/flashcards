#include <iostream>
#include <ncurses.h>
#include <sql.h>
#include <unordered_map>
#include <vector>
#include <string>

struct deck {
    std::string name;
    std::vector<std::string> words;
};

struct language {
    std::string name;
    std::vector<std::string> deck_names;
    std::unordered_map<std::string, deck> decks;
};

struct root {
    std::vector<std::string> lang_names;
    std::unordered_map<std::string, language> languages;
};

int main() {
    sql_db db;
    init_sql(db);

    if (!database_already_exists(db)) {
        generate_schema(db);
        insert_into(db, "version", "idx", 1);
    }

    insert_into(db, "languages", "idx", 1, "name", "Russian");

    int lang_idx = 0;
    auto callback = [](void *data, int ncols, char **col_values, char **col_names) {
        for (int i = 0; i < ncols; i++) {
            std::cout << col_names[i] << ": " << col_values[i] << "\n";
         }

        return 0;
    };

    select_from(db, "select * from languages;", callback);

    //insert_into(db, "decks", "name", "deck 1", "lang_idx", lang_idx);

    close_sql(db);

}
