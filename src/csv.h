#ifndef CSV_H
#define CSV_H

#include <vector>
#include <string>
#include <cstdio>
#include <fstream>
#include <sstream>
// Placeholder CSV parser

inline std::vector<std::string> split_string(std::string line) {
    std::stringstream ss(line);
    std::string word;
    std::vector<std::string> words;
    while (std::getline(ss, word, ',')) {
        words.emplace_back(word);
    }

    return words;
}

inline std::vector<std::vector<std::string>> open_csv(std::string path) {
    std::ifstream fp(path);
    std::vector<std::vector<std::string>> parsed_csv;
    std::string line;

    while (std::getline(fp, line)) {
        parsed_csv.emplace_back(split_string(line));
    }

    return parsed_csv;
}

#endif // CSV_H
