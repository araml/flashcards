#ifndef SQL_H
#define SQL_H

#include <string>
#include <sqlite3.h>
#include <iostream>
#include <sqlite3.h>
#include <tuple>
#include <utility>

struct sql_db {
    std::string name{"database.db"};
    sqlite3 *db{nullptr};
};

struct sql_statement {
    sqlite3_stmt *stmt{nullptr};
};

auto empty_callback = [] (void *, int, char **, char **) { return 0; };

int create_table(sql_db &d, const std::string &table);
int generate_schema(sql_db &d);
int database_already_exists(sql_db &d);
void open_db(sql_db &d);
void close_db(sql_db &d);

template <typename H, typename... T>
void insert_names(std::string &s, H &h, const T &... names) {
    std::cout << "name " << h << std::endl;
    if constexpr (sizeof...(names)) {
        s += std::string(h) + ", ";
        insert_names(s, names...);
    } else {
        s+= std::string(h);
    }
}

template <typename H, typename... T>
void insert_values(sql_statement &s, int idx, const H &h, const T &... names) {
    std::cout << "H " << h << std::endl;
    if constexpr (std::is_integral_v<H>) {
        sqlite3_bind_int(s.stmt, idx, h);
    } else if constexpr (std::is_floating_point_v<H>) {
        sqlite3_bind_double(s.stmt, idx, h);
    } else {
        std::cout << "bind text " << h << " " << std::string(h).size() << std::endl;
        sqlite3_bind_text(s.stmt, idx, h, std::string(h).size(), SQLITE_TRANSIENT);
    }

    if constexpr (sizeof...(names)) {
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
constexpr auto filter_predicate(index_sequence<v>, P predicate) {
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
    s += "?";
    if constexpr (v > 1) {
        s += ", ";
        params<v - 1>(s);
    }
}

template <typename ...T>
void insert_into(sql_db &d, const std::string table_name, T&&... args) {
    if (sizeof...(args) % 2 != 0) {
        /* ERROR not enough parameters */
        return;
    }

    constexpr size_t N = sizeof...(args);
    std::integer_sequence indices = std::make_index_sequence<N>{};
    auto odd_sequence = filter_odd(indices);
    auto even_sequence = filter_even(indices);

    std::string sql_code = "insert or ignore into " + table_name;
    add_parameter_names(sql_code, forward_as_tuple(std::forward<T>(args)...),
                        even_sequence);

    sql_code += " VALUES (";
    params<N / 2>(sql_code);
    sql_code += ") ";

    sql_statement stmt;
    const char *unused;
    std::cout << "prepare error : " <<
                sqlite3_prepare_v2(d.db, sql_code.c_str(), sql_code.size(),
                                    &stmt.stmt, &unused) << std::endl;
    if (!stmt.stmt)
        std::cout << "STATEMENT ERROR " << std::endl;
    std::cout << sqlite3_errmsg(d.db) << std::endl;

    add_parameters(stmt, forward_as_tuple(std::forward<T>(args)...), odd_sequence);

    while (true) {
        auto val = sqlite3_step(stmt.stmt);
        std::cout << "step val : " << val << std::endl;
        if (val == SQLITE_DONE)
            break;
        else
            std::cout << sqlite3_errmsg(d.db) << std::endl;
    }
    sqlite3_finalize(stmt.stmt);
    std::cout << sql_code << std::endl;
}

template <typename T = decltype(empty_callback)>
int select_from(sql_db &d, const std::string &query,
                T callback = empty_callback, void *callback_data = nullptr) {
    char *err;
    if (sqlite3_exec(d.db, query.c_str(), callback, callback_data, &err) == SQLITE_OK) {
        return 1;
    }

    return 0;
}


#endif // SQL_H
