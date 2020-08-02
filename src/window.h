#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <vector>
#include <ncurses.h>

#include <language_structs.h>

void write(int x, int y, const char *fmt, ...);
void cwrite(int x, int y, int width, const std::string &line, unsigned int attr = 0);

void update_filesystem_browser(size_t selected, int width,
                               const std::vector<std::string> &paths);
void update_deck_browser(int width, int height, std::vector<language> &languages);
void update_config(int deck_tree_w, int config_w, int screen_width, int screen_height);

#endif // WINDOW_H
