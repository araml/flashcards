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

#endif // UTILS_H
