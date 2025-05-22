#ifndef _VTL_ADMINISTRATE_USER_H
#define _VTL_ADMINISTRATE_USER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <VTL/utils/VTL_encryptor.h>
#include <libpq-fe.h>
#include <VTL/history_administration/user_administration/VTL_user_model.h>
#include <VTL/utils/db/VTL_db_executor.h>

void VTL_map_result_to_user(PGresult *res, VTL_User *user, int row);
VTL_User VTL_create_user(const char *nickname, const char *password);
VTL_User VTL_get_user_by_id(int id);
VTL_User VTL_get_user_by_nickname(const char *nickname);
int VTL_get_all_users(VTL_User *users, int max_count);
void VTL_update_user_nickname(int id, const char *new_nickname);
void VTL_update_user_password(int id, const char *new_password);
void VTL_delete_user_by_id(int id);
void VTL_delete_user_by_nickname(const char *nickname);
int VTL_user_exists_by_id(int id);
int VTL_user_exists_by_nickname(const char *nickname);
int VTL_count_users();
void VTL_change_user_password(int id, const char *new_password);
int VTL_get_user_id_by_nickname(const char *nickname);
char* VTL_get_user_nickname_by_id(int id);
void VTL_clear_users_table();

#ifdef __cplusplus
}
#endif

#endif