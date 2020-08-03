#include <window.h>
#include <ncurses.h>
#include <vector>
#include <string>
#include <utils.h>
#include <unistd.h>
#include <csv.h>

extern unsigned int reverse_color;
extern std::vector<language> languages;

auto paths = list_dir(fs::current_path());
bool update = true;
size_t fs_selected = 0;
size_t deck_selected = 0;

/* The lang/deck browser can be expanded on runtime to hide all the decks
 * of a single language or hide all the subdecks (TODO) of a deck
 * For example we may have
 * Lang
 *   ~ Deck 1
 *     ~ Sub deck 1
 *     ~ Sub deck 2
 *   ~ Deck 2
 *
 * But this could be seen in the interface as
 *
 * Lang
 * or
 * Lang
 *  ~ Deck 1
 *  ~ Deck 2
 * etc.
 */
size_t deck_browser_size = 0;

void write(int x, int y, const char *fmt, ...) {
    wmove(stdscr, x, y);
    va_list args;
    va_start(args, fmt);
    printw(fmt, args);
    va_end(args);
}

void cwrite(int x, int y, int width, const std::string &line, unsigned int attr) {
    attron(attr);
    std::string l(" ");
    l += line + std::string((size_t)width - line.size() - 1, ' ');
    mvprintw(x, y, l.c_str());
    attroff(attr);
}

static std::vector<word> load_deck(const std::string &path) {
    std::vector<std::vector<std::string>> words = open_csv(path);
    return map<word>(words, [](auto v) { return word{v[0], v[1]}; });
}

std::string remove_slash(std::string s) {
    if (s.size() && s[s.size() - 1] == '/')
        s.pop_back();
    return s;
}

/* For now asume /Language/Deck/Single .csv file
 * TODO: maybe merge many csv files?
 * TODO: if more than one subfolder explore all.
 * TODO: if only one folder and then csvs asks for the language
 * TODO: if .csv only ask for language + deck.
 */
static void add_folder_or_file(const std::string &path) {
    language l;
    l.name = remove_slash(path);
    chdir(path.c_str());
    auto dir = list_dir(fs::current_path());
    deck d;
    d.name = remove_slash(dir[1]);
    chdir(d.name.c_str());
    dir = list_dir(fs::current_path());
    d.words = load_deck(dir[1]);
    l.decks.insert({d.name, d});
    languages.push_back(l);
}

STATE update_filesystem_browser(int c, int width) {
    if (update) {
        print_filesystem_browser(width);
    }

    switch(c) {
        case '1':
            update = true;
            return STATE::DECK;
        case '2':
            break;
        case 'q':
            return STATE::QUIT;
        case 'a':
            add_folder_or_file(paths[fs_selected]);
            break;
        case KEY_DOWN:
            if (fs_selected + 1 < paths.size())
                fs_selected++;
            update = true;
            break;
        case KEY_UP:
            if (fs_selected > 0)
                fs_selected--;
            update = true;
            break;
        case 10:
            if (fs::is_directory(paths[fs_selected])) {
                chdir(paths[fs_selected].c_str());
                paths = list_dir(".");
                fs_selected = 0;
                update = true;
            }
            break;
        default:
            update = false;
    }

    return STATE::FILE_BROWSER;
}

void print_filesystem_browser(int width) {
    clear();

    size_t i = 1;
    cwrite(0, 0, width, "Browser", reverse_color | A_BOLD);
    wbkgdset(stdscr, 0);

    for (auto &word : paths) {
        if (i - 1 == fs_selected)
            cwrite(i, 0, width, word.c_str(), reverse_color | A_BOLD);
        else
            cwrite(i, 0, width, word.c_str());

        i++;
    }

    refresh();
}

static void update_deck_browser_size() {
    size_t sz = 0;
    for (auto &l : languages) {
        sz += 1 + (l.expand_decks ? l.decks.size() : 0);
    }

    deck_browser_size = sz;
}

/*
static void expand_contract_decks(language &l) {
    for (auto &[name, d] : l.decks) {
        d.expanded = !d.expanded;
    }
}
*/

static STATE update_deck_pressed() {
    size_t sz = 0;
    for (auto &l : languages) {
        if (sz == deck_selected) {
            l.expand_decks = !l.expand_decks;
            update_deck_browser_size();
            return STATE::DECK;
        } else {
            sz += 1 + (l.expand_decks ? l.decks.size() : 0);
        }
    }

    return STATE::DECK;
}

STATE update_deck_browser(int c, int width, int height) {
    if (update) {
        update_deck_browser_size();
        print_deck_browser(width, height);
        extern int terminal_width, terminal_height;
        int flash_card_w = terminal_width - width - 1;
        int config_w = flash_card_w / 4;
        print_config(width, config_w, terminal_width, terminal_height);
    }

    switch(c) {
        case '1':
            break;
        case '2':
            update = true;
            fs_selected = 0;
            return STATE::FILE_BROWSER;
        case 'q':
            return STATE::QUIT;
        case KEY_DOWN:
            if (deck_selected + 1 < deck_browser_size) {
                deck_selected++;
            }
            update = true;
            break;
        case KEY_UP:
            if (deck_selected > 0) {
                deck_selected--;
            }
            update = true;
            break;
        case 10:
            if (auto state = update_deck_pressed(); state != STATE::DECK) {
                return state;
            }
            update = true;
            break;
        default:
            update = false;
    }

    return STATE::DECK;
}

static unsigned int is_bold(bool is_bold) {
    if (is_bold)
        return reverse_color | A_BOLD;
    else
        return 0;
}

void print_deck_browser(int width, int height) {
    clear();

    unsigned int blue_thin = COLOR_PAIR(2);
    unsigned int blue_thick = COLOR_PAIR(3);
    cwrite(0, 0, width, "Decks", reverse_color | A_BOLD);
    extern int terminal_width;
    cwrite(0, width + 1, terminal_width - width - 1, "Flashcards",
           reverse_color | A_BOLD);
    size_t k = 1;
    for (auto &l : languages) {
        cwrite(k, 0, width, l.name, is_bold(k - 1 == deck_selected));
        k++;
        if (l.expand_decks) {
            for (auto [deck_name, deck] : l.decks) {
                cwrite(k, 0, width, "  " + deck_name, is_bold(k - 1 == deck_selected));
                k++;
            }
        }
    }

    mvaddch(0, width, blue_thick | ACS_VLINE);
    for (int k = 1; k < height; k++) {
        mvaddch(k, width, blue_thin | ACS_VLINE);
    }

    refresh();
}

std::vector<std::string> config_list {
    "Decks   : 1",
    "Browser : 2",
    "Config  : 3",
    "Add     : A",
    "Select  : Enter",
    "Delete  : Shift + D",
    "Quit    : Esc / q",
};

// why isn't this part of ncurses??????
static void draw_box(int x, int y, int width, int height) {
    mvhline(y, x + 1, ACS_HLINE, width - 1);
    mvhline(y + height, x + 1, ACS_HLINE, width - 1);
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + width, ACS_URCORNER);

    mvvline(y + 1, x, ACS_VLINE, height - 1);
    mvvline(y + 1, x + width, ACS_VLINE, height - 1);
    mvaddch(y + height, x, ACS_LLCORNER);
    mvaddch(y + height, x + width, ACS_LRCORNER);
}


void print_config(int deck_tree_w, int config_w, int screen_width, int screen_height) {
    // TODO: wborder without target window
    //wborder(controls.get_native_window(), 0, 0, 0, 0, 0, 0, 0, 0);
    std::string cfg = "[Controls]";

    int flash_card_w = screen_width - deck_tree_w ;
    int position = ((size_t)config_w - cfg.size()) / 2;
    int y_window = (screen_height - flash_card_w / 9) / 2;
    int x_window =  deck_tree_w  + (flash_card_w - config_w) / 2;

    int cfg_size = (int)config_list.size();

    draw_box(x_window, y_window, config_w + 2, cfg_size + 3);

    write(y_window, x_window + position, cfg.c_str());
    for (size_t i = 0; i < config_list.size(); i++) {
        write(y_window + (int)i + 2, x_window + 3, config_list[i].c_str());
    }
}
