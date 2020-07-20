#include <iostream>
#include <ncurses.h>
#include <sqlite3.h>
#include <tuple>
#include <utility>

struct sql_handle {
    sqlite3 *handle{nullptr};
};

struct sql_statement {
    sqlite3_stmt *stmt{nullptr};
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


template <typename H, typename... T>
void insert_names(std::string &s, H &h, const T &... names) {
    if constexpr (sizeof...(names) > 1) {
        s += std::string(h) + ", ";
        insert_names(s, names...);
    } else {
        s+= std::string(h);
    }
}

template <typename H, typename... T>
void insert_values(sql_statement &s, int idx, const H &h, const T &... names) {
    if constexpr (std::is_integral_v<H>) {
        sqlite3_bind_int(s.stmt, idx, h);
    } else if (std::is_floating_point_v<H>) {
        sqlite3_bind_double(s.stmt, idx, h);
    } else {
        sqlite3_bind_text(s.stmt, idx, h.c_str(), SQLITE_TRANSIENT);
    }

    if constexpr (sizeof...(names) > 1) {
        insert_values(s, ++idx, names...);
    }
}

template <typename T, size_t... indices>
void add_parameter_names(std::string &s, const T &tpl, std::index_sequence<indices...>) {
    s += "(";
    insert_names(s, std::get<indices>(tpl)...);
    s += ") ";
}

template <typename T, size_t... indices>
void add_parameters(sql_statement &s,
                    const T &tpl, std::index_sequence<indices...>) {
    insert_values(s, 1, std::get<indices>(tpl)...);
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

template <size_t v>
constexpr void params(std::string &s) {
    s += "? ";
    if constexpr (v > 1) {
        params<v - 1>(s);
    } else {
        s += ") ";
    }
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

    sql_code += " VALUES (";
    params<N / 2>(sql_code);

    std::cout << "statement " << sql_code << std::endl;
    sql_statement stmt;
    const char *unused;
    sqlite3_prepare_v2(h.handle, sql_code.c_str(), sql_code.size(), &stmt.stmt, &unused);
    add_parameters(stmt, forward_as_tuple(std::forward<T>(args)...), odd_sequence);

    std::cout << "Inserting" << std::endl;
    std::cout << sqlite3_sql(stmt.stmt) << std::endl;
    while (sqlite3_step(stmt.stmt) != SQLITE_DONE) {}
    sqlite3_finalize(stmt.stmt);
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
    init_sql(handle);
    generate_schema(handle);
    insert_into(handle, "version", "idx", 1);
    close_sql(handle);

}
