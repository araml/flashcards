#include <window.h>
#include <ncurses.h>
#include <vector>
#include <string>

extern unsigned int reverse_color;

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

void update_filesystem_browser(size_t selected, int width,
                               const std::vector<std::string> &paths) {
    size_t i = 1;
    cwrite(0, 0, width, "Browser", reverse_color | A_BOLD);
    wbkgdset(stdscr, 0);

    for (auto &word : paths) {
        if (i - 1 == selected)
            cwrite(i, 0, width, word.c_str(), reverse_color | A_BOLD);
        else
            cwrite(i, 0, width, word.c_str());

        i++;
    }

    refresh();
}

void update_deck_browser(int width, int height, std::vector<language> &languages) {
    unsigned int blue_thin = COLOR_PAIR(2);
    unsigned int blue_thick = COLOR_PAIR(3);
    cwrite(0, 0, width, "Decks", reverse_color | A_BOLD);
    size_t k = 1;
    for (auto &l : languages) {
        cwrite(k++, 0, width, l.name);
        for (auto [deck_name, deck] : l.decks) {
            cwrite(k++, 0, width, deck_name);
        }
    }

    mvaddch(0, width, blue_thick | ACS_VLINE);
    for (int k = 1; k < height; k++) {
        mvaddch(k, width, blue_thin | ACS_VLINE);
    }
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


void update_config(int deck_tree_w, int config_w, int screen_width, int screen_height) {
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
