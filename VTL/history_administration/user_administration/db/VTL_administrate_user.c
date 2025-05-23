#include <VTL/history_administration/user_administration/db/VTL_administrate_user.h>
#include <stdlib.h>
#include <string.h>

void VTL_map_result_to_user(PGresult *res, VTL_User *user, int row) {
    user->id = atoi(PQgetvalue(res, row, 0));
    char decrypted_nickname[1024];
    VTL_Decrypt(PQgetvalue(res, row, 1), decrypted_nickname);
    strncpy(user->nickname, decrypted_nickname, sizeof(user->nickname) - 1);
    user->nickname[sizeof(user->nickname) - 1] = '\0';
}

VTL_User VTL_create_user(const char *nickname, const char *password) {
    char encrypted_nickname[1024];
    char encrypted_password[1024];
    VTL_Encrypt(nickname, encrypted_nickname);
    VTL_Encrypt(password, encrypted_password);
    char query[2048];
    snprintf(query, sizeof(query), 
             "CREATE TABLE IF NOT EXISTS users (id SERIAL PRIMARY KEY, nickname VARCHAR(1024), password VARCHAR(1024)); "
             "INSERT INTO users (nickname, password) VALUES ('%s', '%s');", 
             encrypted_nickname, encrypted_password);

    VTL_execute_query(query);
    fprintf(stdout, "\n");
    fprintf(stdout, encrypted_nickname);
    fprintf(stdout, "\n");
    fprintf(stdout, encrypted_password);
    fprintf(stdout, "\n");

    snprintf(query, sizeof(query), 
             "SELECT id, nickname FROM users WHERE nickname = '%s' AND password = '%s';", 
             encrypted_nickname, encrypted_password);
    PGresult *res = VTL_execute_query(query);

    if (res == NULL) {
        fprintf(stderr, "Failed to create user\n");
        return (VTL_User){0};
    }

    VTL_User user;
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        VTL_map_result_to_user(res, &user, i);
    }

    PQclear(res);

    return user;
}

VTL_User VTL_get_user_by_id(int id) {
    char query[256];
    snprintf(query, sizeof(query), "SELECT id, nickname FROM users WHERE id = %d;", id);
    PGresult *res = VTL_execute_query(query);
    VTL_User user = {0};
    if (res && PQntuples(res) > 0) {
        VTL_map_result_to_user(res, &user, 0);
    }
    PQclear(res);
    return user;
}

VTL_User VTL_get_user_by_nickname(const char *nickname) {
    char encrypted_nickname[1024];
    VTL_Encrypt(nickname, encrypted_nickname);
    char query[1024];
    snprintf(query, sizeof(query), "SELECT id, nickname FROM users WHERE nickname = '%s';", encrypted_nickname);
    PGresult *res = VTL_execute_query(query);
    VTL_User user = {0};
    if (res && PQntuples(res) > 0) {
        VTL_map_result_to_user(res, &user, 0);
    }
    PQclear(res);
    return user;
}

int VTL_get_all_users(VTL_User *users, int max_count) {
    char query[] = "SELECT id, nickname FROM users;";
    PGresult *res = VTL_execute_query(query);
    int count = 0;
    if (res) {
        int rows = PQntuples(res);
        for (int i = 0; i < rows && i < max_count; i++) {
            VTL_map_result_to_user(res, &users[i], i);
            count++;
        }
        PQclear(res);
    }
    return count;
}

void VTL_update_user_nickname(int id, const char *new_nickname) {
    char encrypted_nickname[1024];
    VTL_Encrypt(new_nickname, encrypted_nickname);
    char query[1024];
    snprintf(query, sizeof(query), "UPDATE users SET nickname = '%s' WHERE id = %d;", encrypted_nickname, id);
    VTL_execute_query(query);
}

void VTL_update_user_password(int id, const char *new_password) {
    char encrypted_password[1024];
    VTL_Encrypt(new_password, encrypted_password);
    char query[1024];
    snprintf(query, sizeof(query), "UPDATE users SET password = '%s' WHERE id = %d;", encrypted_password, id);
    VTL_execute_query(query);
}

void VTL_delete_user_by_id(int id) {
    char query[256];
    snprintf(query, sizeof(query), "DELETE FROM users WHERE id = %d;", id);
    VTL_execute_query(query);
}

void VTL_delete_user_by_nickname(const char *nickname) {
    char encrypted_nickname[1024];
    VTL_Encrypt(nickname, encrypted_nickname);
    char query[1024];
    snprintf(query, sizeof(query), "DELETE FROM users WHERE nickname = '%s';", encrypted_nickname);
    VTL_execute_query(query);
}

int VTL_user_exists_by_id(int id) {
    char query[256];
    snprintf(query, sizeof(query), "SELECT 1 FROM users WHERE id = %d;", id);
    PGresult *res = VTL_execute_query(query);
    int exists = (res && PQntuples(res) > 0);
    PQclear(res);
    return exists;
}

int VTL_user_exists_by_nickname(const char *nickname) {
    char encrypted_nickname[1024];
    VTL_Encrypt(nickname, encrypted_nickname);
    char query[1024];
    snprintf(query, sizeof(query), "SELECT 1 FROM users WHERE nickname = '%s';", encrypted_nickname);
    PGresult *res = VTL_execute_query(query);
    int exists = (res && PQntuples(res) > 0);
    PQclear(res);
    return exists;
}

int VTL_count_users() {
    char query[] = "SELECT COUNT(*) FROM users;";
    PGresult *res = VTL_execute_query(query);
    int count = 0;
    if (res && PQntuples(res) > 0) {
        count = atoi(PQgetvalue(res, 0, 0));
    }
    PQclear(res);
    return count;
}

void VTL_change_user_password(int id, const char *new_password) {
    VTL_update_user_password(id, new_password);
}

int VTL_get_user_id_by_nickname(const char *nickname) {
    char encrypted_nickname[1024];
    VTL_Encrypt(nickname, encrypted_nickname);
    char query[1024];
    snprintf(query, sizeof(query), "SELECT id FROM users WHERE nickname = '%s';", encrypted_nickname);
    PGresult *res = VTL_execute_query(query);
    int id = 0;
    if (res && PQntuples(res) > 0) {
        id = atoi(PQgetvalue(res, 0, 0));
    }
    PQclear(res);
    return id;
}

char* VTL_get_user_nickname_by_id(int id) {
    char query[256];
    snprintf(query, sizeof(query), "SELECT nickname FROM users WHERE id = %d;", id);
    PGresult *res = VTL_execute_query(query);
    static char nickname[1024];
    nickname[0] = '\0';
    if (res && PQntuples(res) > 0) {
        VTL_Decrypt(PQgetvalue(res, 0, 0), nickname);
    }
    PQclear(res);
    return nickname;
}

void VTL_clear_users_table() {
    char query[] = "DELETE FROM users;";
    VTL_execute_query(query);
}