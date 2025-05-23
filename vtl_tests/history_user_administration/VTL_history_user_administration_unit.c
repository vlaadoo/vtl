#include <vtl_tests/VTL_test_data.h>
#include "VTL/history_administration/user_administration/db/VTL_administrate_user.h"

static int VTL_test_create_user(void) {
    VTL_clear_users_table();
    VTL_User user = VTL_create_user("testuser", "testpass");
    const char* msg = "\nОшибка: пользователь не создан (id == 0)\n";
    return !VTL_test_CheckCondition(user.id != 0, msg) ? 1 : 0;
}

static int VTL_test_get_user_by_id(void) {
    VTL_User user = VTL_create_user("testuser2", "testpass2");
    VTL_User user_by_id = VTL_get_user_by_id(user.id);
    const char* msg = "\nОшибка: не найден пользователь по id\n";
    return !VTL_test_CheckCondition(user_by_id.id == user.id, msg) ? 1 : 0;
}

static int VTL_test_get_user_by_nickname(void) {
    VTL_User user = VTL_create_user("testuser3", "testpass3");
    VTL_User user_by_nick = VTL_get_user_by_nickname("testuser3");
    const char* msg = "\nОшибка: не найден пользователь по nickname\n";
    return !VTL_test_CheckCondition(user_by_nick.id == user.id, msg) ? 1 : 0;
}

static int VTL_test_user_exists_by_id(void) {
    VTL_User user = VTL_create_user("testuser4", "testpass4");
    const char* msg = "\nОшибка: user_exists_by_id не работает\n";
    return !VTL_test_CheckCondition(VTL_user_exists_by_id(user.id), msg) ? 1 : 0;
}

static int VTL_test_user_exists_by_nickname(void) {
    VTL_create_user("testuser5", "testpass5");
    const char* msg = "\nОшибка: user_exists_by_nickname не работает\n";
    return !VTL_test_CheckCondition(VTL_user_exists_by_nickname("testuser5"), msg) ? 1 : 0;
}

static int VTL_test_count_users(void) {
    VTL_clear_users_table();
    VTL_create_user("testuser6", "testpass6");
    int count = VTL_count_users();
    const char* msg = "\nОшибка: VTL_count_users() не возвращает 1\n";
    return !VTL_test_CheckCondition(count == 1, msg) ? 1 : 0;
}

static int VTL_test_delete_user_by_id(void) {
    VTL_clear_users_table();
    VTL_User user = VTL_create_user("testuser7", "testpass7");
    VTL_delete_user_by_id(user.id);
    const char* msg = "\nОшибка: пользователь не удалён по id\n";
    return !VTL_test_CheckCondition(!VTL_user_exists_by_id(user.id), msg) ? 1 : 0;
}

int main(void)
{
    int fail_count = 0;
    fail_count += VTL_test_create_user();
    fail_count += VTL_test_get_user_by_id();
    fail_count += VTL_test_get_user_by_nickname();
    fail_count += VTL_test_user_exists_by_id();
    fail_count += VTL_test_user_exists_by_nickname();
    fail_count += VTL_test_count_users();
    fail_count += VTL_test_delete_user_by_id();
    return fail_count;
}