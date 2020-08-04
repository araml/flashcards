#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

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

std::vector<std::string> list_dir(const fs::path &p);

inline size_t codepoints(const char *line) {
    size_t len = 0;
    size_t codepoints = 0;
    for (; line && line[len] != '\0';) {
        if ((line[len] & 0xF0) == 0xF0) {
            len += 4;
            goto next;
        }

        if ((line[len] & 0xE0) == 0xE0) {
            len += 3;
            goto next;
        }

        if ((line[len] & 0xC0) == 0xC0) {
            len += 2;
            goto next;
        }

        if ((line[len] & 0x80) != 0x80) {
            len++;
            goto next;
        }

next:
        codepoints++;
    }

    return codepoints;
}

#endif // UTILS_H
