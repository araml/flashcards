#include <iostream>
#include <ncurses.h>

#include <sql.h>

int main() {
    sql_db db;
    init_sql(db);

    if (!database_already_exists(db)) {
        generate_schema(db);
        insert_into(db, "version", "idx", 1);
    }

    insert_into(db, "languages", "name", "Russian");
    insert_into(db, "languages", "name", "Norwegian");

    close_sql(db);

}
