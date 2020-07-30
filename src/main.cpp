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

    // We filter out all the hidden files.
    pstr.erase(std::remove_if(pstr.begin(), pstr.end(),
                [] (const string &s) {
                    if (s.size() && s[0] == '.')
                        return true;
                    return false;
                }), pstr.end());

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

    int read_char() {
        return wgetch(w);
    }

    void set_attr(int attr) {
        wattron(w, attr);
    }

    void unset_attr(int attr) {
        wattroff(w, attr);
    }

private:
    WINDOW *w;
    int width, height;
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
    keypad(stdscr, true);
    #define REVERSE_COLOR 1

    window search_window = window(width, height);
    window deck_w = window(width, height);

    size_t selected = 0;

    //attron(COLOR_PAIR(0));

    auto paths = list_dir(fs::current_path());

    bool deck_window = true;
    bool quit = false;
    int c;
    while (!quit) {
        size_t i = 0;

        if (!deck_window) {
            for (auto &w : paths) {
                if (i == selected) {
                    search_window.set_attr(COLOR_PAIR(REVERSE_COLOR));
                }

                search_window.write(i, 0, w.c_str());

                if (i == selected) {
                    search_window.unset_attr(COLOR_PAIR(REVERSE_COLOR));
                }

                i++;
            }
            c = search_window.read_char();
        } else {
            deck_w.write(i, 0, "Decks");
            c = deck_w.read_char();
        }


        switch(c) {
            case '1':
                deck_window = true;
                search_window.clear();

                break;
            case '2':
                deck_window = false;
                deck_w.clear();
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
