#include <iostream>
#include <ncurses.h>
#include <sql.h>
#include <string>
#include <filesystem>
#include <unistd.h>

#include <sys/ioctl.h>
#include <signal.h>

#include <window.h>
#include <csv.h>
#include <utils.h>

namespace fs = std::filesystem;



std::vector<language> languages;

bool cmp_str(const std::string &s1, const std::string &s2) {
    return s1 == s2;
}

void sig_winch([[gnu::unused]] int irq) {
    return;
}

int height, width;

std::vector<word> load_deck(const std::string &path) {
    std::vector<std::vector<std::string>> words = open_csv(path);
    return map<word>(words, [](auto v) { return word{v[0], v[1]}; });
}

std::vector<std::string> control_str {
    "Decks   : 1",
    "Browser : 2",
    "Config  : 3",
    "Add     : A",
    "Select  : Enter",
    "Delete  : Shift + D",
    "Quit    : Esc / q",
};

/* For now asume /Language/Deck/Single .csv file
 * TODO: maybe merge many csv files?
 * TODO: if more than one subfolder explore all.
 * TODO: if only one folder and then csvs asks for the language
 * TODO: if .csv only ask for language + deck.
 */
void add_folder_or_file(const std::string &path) {
    language l;
    l.name = path;
    chdir(path.c_str());
    auto dir = list_dir(fs::current_path());
    deck d;
    d.name = dir[1];
    chdir(d.name.c_str());
    dir = list_dir(fs::current_path());
    d.words = load_deck(dir[1]);
    l.decks.insert({d.name, d});
    languages.push_back(l);
}

unsigned int reverse_color = COLOR_PAIR(1);
unsigned int blue_thin = COLOR_PAIR(2);
unsigned int blue_thick = COLOR_PAIR(3);

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
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLUE, -1);
    init_pair(3, COLOR_BLUE, COLOR_BLUE);
    keypad(stdscr, true);
    //#define REVERSE_COLOR 1

    int deck_tree_w = width / 3;
    int flash_card_w = width - deck_tree_w - 1;
    int control_w = flash_card_w / 4;

    nodelay(stdscr, true);


    //attron(COLOR_PAIR(0));

    auto paths = list_dir(fs::current_path());
    size_t selected = 0;

    bool deck_window = true;
    bool quit = false;
    int c;
    deck d;
    while (!quit) {
        if (!deck_window) {
            update_filesystem_browser(selected, width, paths);
        } else {
            update_deck_browser(deck_tree_w, languages);

            mvaddch(0, deck_tree_w, blue_thick | ACS_VLINE);
            for (int k = 1; k < height; k++) {
                mvaddch(k, deck_tree_w, blue_thin | ACS_VLINE);
            }

            // TODO: wborder without target window
            //wborder(controls.get_native_window(), 0, 0, 0, 0, 0, 0, 0, 0);
            std::string cfg = "[Controls]";
            int position = ((size_t)control_w - cfg.size()) / 2;
            int y_window = (height - flash_card_w / 9) / 2;
            int x_window =  deck_tree_w + (flash_card_w - control_w) / 2;
            cwrite(y_window, x_window + position, control_w, cfg);


            for (size_t i = 0; i < control_str.size(); i++) {
                cwrite(y_window + (int)i + 1, x_window + 3, control_w, control_str[i].c_str());
            }

            refresh();
        }

        c = getch(); // deck_tree.read_char();

        switch(c) {
            case '1':
                deck_window = true;
                clear();
                break;
            case '2':
                deck_window = false;
                break;
            case 'q':
                quit = true;
                break;
            case 'a':
                add_folder_or_file(paths[selected]);
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
                if (fs::is_directory(paths[selected])) {
                    chdir(paths[selected].c_str());
                    paths = list_dir(".");
                    selected = 0;
                } else {
                    //d = load_deck(paths[selected]);
                }
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
