#include <iostream>
#include <ncurses.h>
#include <sqlite3.h>
#include <tuple>
#include <utility>

struct sql_handle {
    sqlite3 *handle{nullptr};
};

void create_table(sql_handle &h, const std::string &table) {
    sqlite3_stmt *stmt;
    const char *unused;
    sqlite3_prepare_v2(h.handle, table.c_str(), table.size(), &stmt, &unused);
    while (sqlite3_step(stmt) != SQLITE_DONE) {}
    sqlite3_finalize(stmt);
}

void generate_schema(sql_handle &h) {
    // TODO: Check if db exists already.
    std::string version = "create table version (idx int primary key not null);";
    std::string language = "create table languages (idx int primary key not null,"
                           "name text not null);";
    std::string decks = "create table decks (idx int primary key not null,"
                        "lang_idx int, name text not null,"
                        "foreign key(lang_idx) references languages(idx));";

    std::string words = "create table words (idx int primary key not null,"
                                            "word text not null);";

    std::string deck_word = "create table deck_word (word_idx int, deck_idx int,"
                            "foreign key(word_idx) references words(idx),"
                            "foreign key(deck_idx) references decks(idx));";

    create_table(h, version);
    create_table(h, language);
    create_table(h, decks);
    create_table(h, words);
    create_table(h, deck_word);
}

template <typename T>
void insert_names(std::string &s, const T &name) {
    s += std::string(name);
}

template <typename H, typename... T>
void insert_names(std::string &s, H &h, const T &... names) {
    s += std::string(h) + ", ";
    insert_names(s, names...);
}

template <typename T>
void insert_values(std::string &s, const T &name) {
    s += std::string(name);
}

template <typename H, typename... T>
void insert_values(std::string &s, const H &h, const T &... names) {
    s += std::string(h) + ", ";
    insert_values(s, names...);
}

template <typename T, size_t... indices>
void add_parameter_names(std::string &s, const T &tpl, std::index_sequence<indices...>) {
    s += "(";
    insert_names(s, std::get<indices>(tpl)...);
    s += ") ";
}

template <typename T, size_t... indices>
void add_parameters(std::string &s, const T &tpl, std::index_sequence<indices...>) {
    insert_values(s, std::get<indices>(tpl)...);
}


using std::forward_as_tuple;
using std::string;
using std::index_sequence;

template <size_t... A, size_t... B>
index_sequence<A..., B...> operator+(index_sequence<A...>, index_sequence<B...>) {
    return {};
}

template <size_t v, typename P>
constexpr auto filter_predicate(index_sequence<v>H, P predicate) {
    if constexpr (predicate(v)) {
        return index_sequence<v>{};
    } else {
        return index_sequence<>{};
    }
}

template <size_t... indexes>
constexpr auto filter_even(index_sequence<indexes...>) {
    return (filter_predicate(index_sequence<indexes>{},
            [] (size_t i) constexpr { return !(i % 2); }) + ...);
}

template <size_t... indexes>
constexpr auto filter_odd(index_sequence<indexes...>) {
    return (filter_predicate(index_sequence<indexes>{},
            [] (size_t i) constexpr { return i % 2; }) + ...);
}

template <typename ...T>
void insert_into(sql_handle &h, const std::string table_name, T&&... args) {
    if (sizeof...(args) % 2 != 0) {
        /* ERROR not enough parameters */
        return;
    }

    constexpr size_t N = sizeof...(args);
    std::integer_sequence indices = std::make_index_sequence<N>{};
    auto odd_sequence = filter_odd(indices);
    auto even_sequence = filter_even(indices);

    std::string sql_code = "insert into " + table_name;
    add_parameter_names(sql_code, forward_as_tuple(std::forward<T>(args)...),
                        even_sequence);
    add_parameters(sql_code, forward_as_tuple(std::forward<T>(args)...),
                   odd_sequence);
    std::cout << sql_code << std::endl;
}


void init_sql(sql_handle &h) {
    std::cout << "Open\n";
    std::cout << sqlite3_open("database.db", &h.handle) << std::endl;
}

void close_sql(sql_handle &h) {
    std::cout << "closing sql" << std::endl;
    sqlite3_close(h.handle);
}


int main() {
    sql_handle handle;
    insert_into(handle, "languages", "Param name 1", "Param 1", "Param name 2", "Param 2");

    init_sql(handle);
    generate_schema(handle);
    close_sql(handle);

}
