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
void update_deck_browser(int width, std::vector<language> &languages);


#endif // WINDOW_H
