#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <vector>

#include <ncurses.h>

#include <language_structs.h>

enum class STATE {
    DECK,
    FLASH_CARD,
    FILE_BROWSER,
    CONFIG,
    QUIT,
};

void write(int x, int y, const char *fmt, ...);
void cwrite(int x, int y, int width, const std::string &line, unsigned int attr = 0);

STATE update_filesystem_browser(int c, int width);
STATE update_deck_browser(int c, int width, int height);

void print_filesystem_browser(int width);
void print_deck_browser(int width, int height);
void update_config(int deck_tree_w, int config_w, int screen_width, int screen_height);

#endif // WINDOW_H
