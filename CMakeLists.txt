cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(BUILD_SHARED_LIBS ON)

project(VTL C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(AppName ${CMAKE_PROJECT_NAME})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app)


set(VTL_CMAKE_UTILS_SRC_FILES

        VTL/utils/VTL_time.c
        VTL/utils/VTL_file.c
        VTL/utils/VTL_encryptor.c

        VTL/utils/log/VTL_console_log.c

        VTL/utils/db/VTL_db_credentals.c


        VTL/utils/db/VTL_db_connection.c
        VTL/utils/db/VTL_db_executor.c
)

set(VTL_CMAKE_HISTORY_ADMINISTRATION_SRC_FILES

        VTL/history_administration/VTL_history_administration.c
        VTL/history_administration/user_administration/db/VTL_administrate_user.c

)


set(VTL_CMAKE_USER_SRC_FILES

        VTL/user/VTL_user_data.c
        VTL/user/history/VTL_user_history_data.c
        VTL/user/history/db/VTL_user_history_Save.c

)

set(VTL_CMAKE_PUBLICATION_TEXT_SRC_FILES

        VTL/publication/text/VTL_publication_text_data.c
        VTL/publication/text/VTL_publication_text_op.c
        VTL/publication/text/VTL_publication_markup_text_flags_check.c
        VTL/publication/text/infra/VTL_publication_text_read.c
        VTL/publication/text/infra/VTL_publication_text_write.c

)

set(VTL_CMAKE_PUBLICATION_SRC_FILES

        ${VTL_CMAKE_PUBLICATION_TEXT_SRC_FILES}
        VTL/content_platform/tg/VTL_content_platform_tg_net.c
        VTL/content_platform/VTL_content_platform_publication.c

        VTL/publication/VTL_publication.c

)


set(VTL_CMAKE_MEDIA_CONTAINER_AUDIO_SRC_FILES

        VTL/media_container/audio/VTL_audio_data.c
        VTL/media_container/audio/infra/VTL_audio_read.c
        VTL/media_container/audio/infra/VTL_audio_write.c

)

set(VTL_CMAKE_MEDIA_CONTAINER_VIDEO_SRC_FILES

        VTL/media_container/video/VTL_video_data.c
)

set(VTL_CMAKE_MEDIA_CONTAINER_SRC_FILES

        ${VTL_CMAKE_MEDIA_CONTAINER_AUDIO_SRC_FILES}
        ${VTL_CMAKE_MEDIA_CONTAINER_VIDEO_SRC_FILES}

)

set(VTL_CMAKE_CONTENT_PLATFORM_TG_SRC_FILES

        VTL/content_platform/tg/media_container_params/VTL_content_platform_tg_params_audio.c
        VTL/content_platform/tg/media_container_params/VTL_content_platform_tg_params_sub.c
        VTL/content_platform/tg/media_container_params/VTL_content_platform_tg_params_video.c
        VTL/content_platform/infra/text/VTL_text_configs_for_gen_init.c
)

set(VTL_CMAKE_CONTENT_PLATFORM_SRC_FILES

        ${VTL_CMAKE_CONTENT_PLATFORM_TG_SRC_FILES}
        VTL/content_platform/VTL_content_platform_flags_check.c
        VTL/content_platform/infra/audio/VTL_audio_gen.c
        VTL/content_platform/infra/audio/VTL_audio_configs_for_gen_init.c
        VTL/content_platform/infra/text/VTL_publication_text_gen.c
        VTL/content_platform/VTL_content_platform_publication.c
)

add_executable(${AppName}
        ${VTL_CMAKE_UTILS_SRC_FILES}
        ${VTL_CMAKE_HISTORY_ADMINISTRATION_SRC_FILES}
        ${VTL_CMAKE_USER_SRC_FILES}
        ${VTL_CMAKE_PUBLICATION_SRC_FILES}
        ${VTL_CMAKE_MEDIA_CONTAINER_SRC_FILES}
        ${VTL_CMAKE_CONTENT_PLATFORM_SRC_FILES}

        main.c)


## Так добавлять в проект сторонние библиотеки
# add_subdirectory(external_sources/openssl)
# target_link_libraries(${AppName} PRIVATE openssl)

# include(ExternalProject)

#ExternalProject_Add(
#        openssl_external
#        URL https://www.openssl.org/source/openssl-3.0.14.tar.gz
#        CONFIGURE_COMMAND perl Configure VC-WIN64A --prefix=<INSTALL_DIR>
#        BUILD_COMMAND set CL=/FS && nmake
#        INSTALL_COMMAND nmake install
#        BUILD_IN_SOURCE 1
#        ENVIRONMENT "CL=/FS"
#)
#
#
#add_dependencies(${AppName} openssl_external)
#target_include_directories(${AppName} PRIVATE ${CMAKE_BINARY_DIR}/_deps/openssl_external-install/include)
#target_link_directories(${AppName} PRIVATE ${CMAKE_BINARY_DIR}/_deps/openssl_external-install/lib)
#target_link_libraries(${AppName} PRIVATE libssl libcrypto)



#ExternalProject_Add(
#        postgresql_libpq
#        URL https://ftp.postgresql.org/pub/source/v13.14/postgresql-13.14.tar.gz
#        SOURCE_SUBDIR src/interfaces/libpq
#        CONFIGURE_COMMAND ""
#        BUILD_COMMAND nmake /f win32.mak
#        INSTALL_COMMAND ""
#        BUILD_IN_SOURCE 1
#)
#
#add_dependencies(${AppName} postgresql_libpq)
#target_include_directories(${AppName} PRIVATE ${CMAKE_BINARY_DIR}/_deps/postgresql_libpq-src/include)
#target_link_libraries(${AppName} PRIVATE ${CMAKE_BINARY_DIR}/_deps/postgresql_libpq-src/interfaces/libpq/Release/libpq.lib)



# Указываем наши пути
target_include_directories(${AppName} PRIVATE ${CMAKE_SOURCE_DIR}/external/postgres/include)
target_link_libraries(${AppName} "${CMAKE_SOURCE_DIR}/external/postgres/lib/libpq.lib")

# Копируем DLL рядом с exe после сборки
add_custom_command(TARGET ${AppName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/external/postgres/bin/libpq.dll
        $<TARGET_FILE_DIR:${AppName}>
)

target_include_directories(${AppName}
        PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/VTL
        ${CMAKE_SOURCE_DIR}/VTL/history_administration
        ${CMAKE_SOURCE_DIR}/VTL/user
        ${CMAKE_SOURCE_DIR}/VTL/user/history
        ${CMAKE_SOURCE_DIR}/VTL/publication
        ${CMAKE_SOURCE_DIR}/VTL/content_platform
        ${CMAKE_SOURCE_DIR}/VTL/media_container
)
enable_testing()

add_subdirectory(vtl_tests)


