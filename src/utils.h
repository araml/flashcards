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

#endif // UTILS_H
