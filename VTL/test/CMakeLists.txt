set(VTL_CMAKE_TESTS_APP_NAME VTL_Tests)

set(VTL_CMAKE_TESTS_RUN VTL_tests_Run)

add_executable(${VTL_CMAKE_TESTS_APP_NAME} VTL_test_data.c VTL_test.c)

target_include_directories(${VTL_CMAKE_TESTS_APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR})

add_test(NAME ${VTL_CMAKE_TESTS_RUN} COMMAND ${VTL_CMAKE_TESTS_APP_NAME})

# Добавлено демо для прожига субтитров с поддержкой стилей и парсинга JSON
add_executable(VTL_sub_style_demo
    main_sub_style_demo.c
    ../media_container/sub/VTL_sub_convert.c
    ../media_container/sub/VTL_sub_style.c
    ../media_container/sub/VTL_sub_parse.c
    ../media_container/sub/VTL_sub_burn.c
    ../media_container/sub/VTL_sub_read.c
    ../media_container/sub/VTL_sub_write.c
    ../media_container/sub/VTL_sub_data.c
    ../../external/parson/parson.c
)
target_include_directories(VTL_sub_style_demo PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/VTL
    ${CMAKE_SOURCE_DIR}/VTL/media_container/sub
    ${CMAKE_SOURCE_DIR}/external/parson
)
target_link_libraries(VTL_sub_style_demo PRIVATE m z bz2 lzma ffmpeg)
add_dependencies(VTL_sub_style_demo ffmpeg_ext)