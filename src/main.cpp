#include <iostream>
#include <ncurses.h>
#include <sql.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <filesystem>

#include <csv.h>

namespace fs = std::filesystem;

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

bool cmp_str(const std::string &s1, const std::string &s2) {
    return s1 == s2;
}

int main() {
    for (auto &p : fs::directory_iterator("..", fs::directory_options::follow_directory_symlink)) {
        std::cout << p.path().string();
        if (fs::is_directory(p))
            std::cout << "/";
        std::cout << std::endl;
    }

    auto www = open_csv("rs.csv");
    for (auto ww : www) {
        for (auto w : ww) {
            std::cout << w;
        }
        std::cout << std::endl;
    }

    return 0;

    sql_db db;
    open_db(db);

    if (!database_already_exists(db)) {
        generate_schema(db);
        insert_into(db, "version", "idx", 1);
    }

    insert_into(db, "languages", "idx", 1, "name", "Russian");

    int lang_idx = 1337;

    auto callback = [](void *data, int ncols, char **col_values, char **col_names) {
        std::cout << std::endl;
        std::cout << "Callback " << std::endl;
        for (int i = 0; i < ncols; i++) {
            std::cout << col_names[i] << ": " << col_values[i] << "\n";
            if (cmp_str(col_names[i], "idx")) {
                *(int *)data = atoi(col_values[i]);
            }
        }

        return 0;
    };

    select_from(db, "select * from languages where name='Russian';", callback, &lang_idx);

    std::cout << "idx " << lang_idx << std::endl;;


    insert_into(db, "decks", "lang_idx", lang_idx, "name", "Chapter 1");


    int deck_idx = 4444;
    select_from(db, "select * from decks where name='Chapter 1';", callback, &deck_idx);
    std::cout << "deck idx " << deck_idx << std::endl;

    insert_into(db, "words", "word", "здравствуйте");

    int word_idx = 9999;

    select_from(db, "select * from words;", callback, &word_idx);
    std::cout << "deck idx " << word_idx << std::endl;

    insert_into(db, "deck_word", "word_idx", word_idx, "deck_idx", deck_idx);

    close_db(db);

}
