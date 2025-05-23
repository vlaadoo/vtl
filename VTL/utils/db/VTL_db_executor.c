#include <VTL/utils/db/VTL_db_executor.h>


PGresult* VTL_execute_query(char *query) {
    VTL_db_Credentals db_cred = {
        .host = "localhost",
        .port = "5432",
        .user = "your_user",
        .password = "your_password",
        .dbname = "your_db"
    };
    PGconn *conn = VTL_connect_to_db(&db_cred);
    PGresult *res = PQexec(conn, query);
    // if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    //     fprintf(stderr, "Query failed: %s", PQerrorMessage(conn));
    //     PQclear(res);
    //     PQfinish(conn);
    //     return NULL;
    // }
    PQfinish(conn);
    return res;
}