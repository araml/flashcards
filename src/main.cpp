#include <iostream>
#include <ncurses.h>
#include <sql.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <filesystem>
#include <unistd.h>

#include <sys/ioctl.h>
#include <signal.h>

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

void sig_winch([[gnu::unused]] int irq) {
    return;
}

int height, width;

std::vector<std::string> list_dir(const string& path) {
    std::vector<std::string> paths{".", ".."};
    for (auto &p : fs::directory_iterator(path)) {
        paths.emplace_back(p.path().string());
        if (fs::is_directory(p))
            paths.back() += "/";
    }

    return paths;
}

int main() {
    struct winsize max;
    ioctl(1, TIOCGWINSZ, &max);
    height = max.ws_row;
    width = max.ws_col;
    setlocale(LC_ALL, "");
    initscr();
    curs_set(0);
    noecho();
    cbreak();
    signal(SIGWINCH, sig_winch);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    keypad(stdscr, TRUE);
    #define REVERSE_COLOR 1

    WINDOW *w_search = newwin(height, width, 0, 0);
    WINDOW *w_decks = newwin(height, width, 0, 0);

    size_t selected = 0;

    attron(COLOR_PAIR(0));

    auto paths = list_dir(".");

    bool deck_window = true;
    bool quit = false;
    int c;
    while (!quit) {
        size_t i = 0;

        if (!deck_window) {
            for (auto &w : paths) {

                if (i == selected) {
                    wattron(w_decks, COLOR_PAIR(REVERSE_COLOR));
                }

                mvwprintw(w_search, i, 0, w.c_str());

                if (i == selected) {
                    wattroff(w_decks, COLOR_PAIR(REVERSE_COLOR));
                }

                i++;
            }
            c = wgetch(w_search);
        } else {
            mvwprintw(w_decks, i, 0, "Decks");
            c = wgetch(w_decks);
        }


        switch(c) {
            case '1':
                deck_window = true;
                wclear(w_search);
                clear();
                break;
            case '2':
                deck_window = false;
                wclear(w_decks);
                clear();
                break;
            case 'q':
                quit = true;
                break;
            case KEY_DOWN:
                if (selected + 1 < paths.size())
                    selected++;
                break;
            case KEY_UP:
                if (selected > 0)
                    selected--;
                break;
            case 10:
                chdir(paths[selected].c_str());
                paths = list_dir(".");
                selected = 0;
                i = 0;
                clear();
                break;
        }
    }

    endwin();
}

/*

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
    */
