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

void update_deck_browser(int width, std::vector<language> &languages) {
    cwrite(0, 0, width, "Decks", reverse_color | A_BOLD);
    size_t k = 1;
    for (auto &l : languages) {
        cwrite(k++, 0, width, l.name);
        for (auto [deck_name, deck] : l.decks) {
            cwrite(k++, 0, width, deck_name);
        }
    }
}
