#ifndef LANGUAGE_STRUCTS_H
#define LANGUAGE_STRUCTS_H

#include <vector>
#include <string>
#include <unordered_map>

struct word {
    std::string untranslated;
    std::string translated;
    /* TODO: maybe extende later for conjugations etc?*/
};

struct deck {
    bool expanded{false};
    std::string name;
    std::vector<word> words;
};

struct language {
    std::string name;
    std::vector<std::string> deck_names;
    std::unordered_map<std::string, deck> decks;
};

#endif // LANGUAGE_STRUCTS_H
