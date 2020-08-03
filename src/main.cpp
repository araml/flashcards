#include <iostream>
#include <ncurses.h>
#include <sql.h>
#include <string>
#include <filesystem>
#include <chrono>
#include <thread>

#include <unistd.h>

#include <sys/ioctl.h>
#include <signal.h>


#include <window.h>
#include <csv.h>
#include <utils.h>

namespace fs = std::filesystem;

bool window_size_changed = false;
std::vector<language> languages;
int terminal_height, terminal_width;

void sig_winch([[gnu::unused]] int irq) {
    window_size_changed = true;
}

void update_term_size() {
    struct winsize size;
    ioctl(1, TIOCGWINSZ, &size);
    resizeterm(size.ws_row, size.ws_col);
    terminal_height = size.ws_row;
    terminal_width = size.ws_col;
}


unsigned int reverse_color = COLOR_PAIR(1);

int main() {
    update_term_size();
    setlocale(LC_ALL, "");
    initscr();
    curs_set(0);
    noecho();
    cbreak();
    signal(SIGWINCH, sig_winch);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLUE, -1);
    init_pair(3, COLOR_BLUE, COLOR_BLUE);
    keypad(stdscr, true);
    //#define REVERSE_COLOR 1

    //int flash_card_w = terminal_width - deck_tree_w - 1;
    //int config_w = flash_card_w / 4;

    nodelay(stdscr, true);


    //attron(COLOR_PAIR(0));

    bool quit = false;
    int c = 0;
    deck d;

    STATE state = STATE::DECK;

    while (!quit) {
        if (window_size_changed) {
            window_size_changed = false;
            update_term_size();
            extern bool update;
            update = true;
        }

        c = getch();
        if (state == STATE::FILE_BROWSER) {
            state = update_filesystem_browser(c, terminal_width);
        } else if (state == STATE::DECK) {
            state = update_deck_browser(c, terminal_width / 3, terminal_height);
        } else if (state == STATE::QUIT) {
            break;
        } else {

        }

        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds{25});
    }

    endwin();
}

/*
 *bool cmp_str(const std::string &s1, const std::string &s2) {
    return s1 == s2;
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

*/
