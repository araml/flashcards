#include <iostream>
#include <ncurses.h>
#include <sql.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <unistd.h>

#include <sys/ioctl.h>
#include <signal.h>

#include <csv.h>

namespace fs = std::filesystem;

struct word {
    std::string untranslated;
    std::string translated;
    /* TODO: maybe extende later for conjugations etc?*/
};

struct deck {
    std::string name;
    std::vector<word> words;
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

template <typename T, typename Predicate>
std::vector<T> filter(const std::vector<T> &v, Predicate p) {
    std::vector<T> filtered;
    std::copy_if(v.begin(), v.end(), std::back_inserter(filtered), p);
    return filtered;
}

template <typename R, typename T, typename Predicate>
std::vector<R> map(const std::vector<T> &v, Predicate p) {
    std::vector<R> mapd(v.size());
    std::transform(v.begin(), v.end(), mapd.begin(), p);
    return mapd;
}

std::vector<std::string> list_dir(const fs::path &p) {
    fs::directory_iterator it = fs::directory_iterator(p);

    // Sort by directories first and common files later
    std::vector<fs::path> paths(begin(it), end(it));
    std::sort(paths.begin(), paths.end(), [](const fs::path &p1,
                                             const fs::path &p2) {
                return (fs::is_directory(p1) && fs::is_directory(p2) &&
                    (p1 < p2)) || (fs::is_directory(p1) && !fs::is_directory(p2));
            });

    // Agregamos el "/"
    std::vector<std::string> pstr = map<std::string>(paths,
            [] (const fs::path &p1) {
                return fs::is_directory(p1) ? p1.filename().string() + "/"
                : p1.filename().string();
            });

    pstr = filter(pstr,
            [] (const string &s) {
                return fs::is_directory(s) || s.ends_with(".csv");
            });

    pstr.insert(pstr.begin(), "../");
    return pstr;
}

class window {
public:
    window(int width, int height)
        : width(width),
          height(height)
    {
        w = newwin(height, width, 0, 0);
        keypad(w, true);
    }

    ~window() {
        delwin(w);
    }

    void resize(int, int) { }

    void clear() {
        wclear(w);
    }

    void refresh() {
        wrefresh(w);
    }

    void write(int x, int y, const char *fmt, ...) {
        wmove(w, x, y);
        va_list args;
        va_start(args, fmt);
        vw_printw(w, fmt, args);
        va_end(args);
    }

    void cwrite(int x, int y, std::string line, unsigned int attr = 0) {
        set_attr(attr);
        std::string l(" ");
        l += line + std::string((size_t)width - line.size() - 1, ' ');
        mvwprintw(w, x, y, l.c_str());
        unset_attr(attr);
    }

    int read_char() {
        return wgetch(w);
    }

    void set_attr(unsigned int attr) {
        wattron(w, attr);
    }

    void unset_attr(unsigned int attr) {
        wattroff(w, attr);
    }

    WINDOW *get_native_window() { return w; }

private:
    WINDOW *w;
    int width, height;
};


deck load_deck(const std::string &path) {
    deck d;
    d.name = path;
    std::vector<std::vector<std::string>> words = open_csv(path);
    d.words = map<word>(words, [](auto v) { return word{v[0], v[1]}; });
    return d;
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

    unsigned int reverse_color = COLOR_PAIR(1);
    unsigned int blue_thin = COLOR_PAIR(2);
    unsigned int blue_thick = COLOR_PAIR(3);

    window search_window = window(width, height);
    int deck_tree_w = width / 3;
    int flash_card_w = width - deck_tree_w - 1;
    window deck_tree = window(deck_tree_w, height);
    window flash_card = window(flash_card_w, height);
    int control_w = flash_card_w / 4;
    window controls = window(control_w, flash_card_w / 9);

    nodelay(stdscr, true);
    size_t selected = 0;

    //attron(COLOR_PAIR(0));

    auto paths = list_dir(fs::current_path());

    bool deck_window = true;
    bool quit = false;
    int c;
    deck d;
    while (!quit) {
        size_t i = 1;
        if (!deck_window) {
            search_window.cwrite(0, 0, "Browser", reverse_color | A_BOLD);
            wbkgdset(search_window.get_native_window(), 0);
            for (auto &w : paths) {
                if (i - 1 == selected)
                    search_window.cwrite(i, 0, w.c_str(), reverse_color | A_BOLD);
                else
                    search_window.cwrite(i, 0, w.c_str());

                i++;
            }


            refresh();
            wrefresh(search_window.get_native_window());
            c = getch(); //search_window.read_char();
        } else {
            deck_tree.cwrite(0, 0, "Decks", reverse_color | A_BOLD);
            size_t k = 1;
            for (auto w : d.words) {
                deck_tree.write(k, 0, std::string(w.untranslated + " " + w.translated).c_str());
                k++;
            }

            mvaddch(0, deck_tree_w, blue_thick | ACS_VLINE);
            for (int k = 1; k < height; k++) {
                mvaddch(k, deck_tree_w, blue_thin | ACS_VLINE);
            }

            wborder(controls.get_native_window(), 0, 0, 0, 0, 0, 0, 0, 0);
            std::string cfg = "[Controls]";
            int position = ((size_t)control_w - cfg.size()) / 2;
            controls.write(0, position, cfg.c_str());
            mvprintw(30, 30, "%d", (flash_card_w - control_w) / 2);
            mvwin(controls.get_native_window(), (height - flash_card_w / 9) / 2,
                                                deck_tree_w + (flash_card_w - control_w) / 2);

            for (size_t i = 0; i < control_str.size(); i++) {
                controls.write(i + 1, 3, control_str[i].c_str());
            }

            refresh();
            wrefresh(deck_tree.get_native_window());
            wrefresh(controls.get_native_window());
            c = getch(); // deck_tree.read_char();
        }


        switch(c) {
            case '1':
                deck_window = true;
                clear();
                search_window.clear();
                break;
            case '2':
                deck_window = false;
                controls.clear();
                deck_tree.clear();
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
                if (fs::is_directory(paths[selected])) {
                    chdir(paths[selected].c_str());
                    paths = list_dir(".");
                    selected = 0;
                    i = 0;
                } else {
                    d = load_deck(paths[selected]);
                }
                search_window.clear();
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
