set(VTL_CMAKE_HISTORY_USER_ADMIN_TESTS_APP_NAME VTL_tests_HistoryUserAdmin)

set(VTL_CMAKE_HISTORY_USER_ADMIN_TESTS_RUN VTL_tests_history_user_admin_Run)

add_executable(${VTL_CMAKE_HISTORY_USER_ADMIN_TESTS_APP_NAME}
                ../VTL_test_data.c
                VTL_history_user_administration_unit.c)


add_library(VTL_user_admin STATIC
        ${CMAKE_SOURCE_DIR}/VTL/history_administration/user_administration/db/VTL_administrate_user.c
)

target_sources(VTL_user_admin PRIVATE
        ${CMAKE_SOURCE_DIR}/VTL/utils/VTL_encryptor.c
        ${CMAKE_SOURCE_DIR}/VTL/utils/db/VTL_db_executor.c
        ${CMAKE_SOURCE_DIR}/VTL/utils/db/VTL_db_connection.c
)

target_include_directories(VTL_user_admin PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/external/postgres/include
)


target_include_directories(${VTL_CMAKE_HISTORY_USER_ADMIN_TESTS_APP_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/external/postgres/include
)

target_link_libraries(${VTL_CMAKE_HISTORY_USER_ADMIN_TESTS_APP_NAME} PRIVATE VTL_user_admin)


add_test(NAME ${VTL_CMAKE_HISTORY_USER_ADMIN_TESTS_RUN} COMMAND ${VTL_CMAKE_HISTORY_USER_ADMIN_TESTS_APP_NAME})
