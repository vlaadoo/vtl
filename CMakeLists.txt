cmake_minimum_required(VERSION 3.25.0)

project(VTL C)

# include(ExternalProject)
# 
# ExternalProject_Add(
#     ffmpeg_ext
#     SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/ffmpeg
#     CONFIGURE_COMMAND ./configure --disable-doc --disable-programs --enable-ffmpeg --disable-ffplay --disable-ffprobe --disable-static --enable-shared --prefix=${CMAKE_SOURCE_DIR}/external/ffmpeg_build
#     BUILD_COMMAND make -j$(nproc)
#     INSTALL_COMMAND make install
#     BUILD_IN_SOURCE 1
# )

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Add compiler flags
add_compile_options(-Wno-implicit-function-declaration)

# Add system include paths
include_directories(SYSTEM /usr/include)

include(cmake/FFmpeg.cmake)

set(AppName ${CMAKE_PROJECT_NAME})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app )


set(VTL_CMAKE_UTILS_SRC_FILES 

    VTL/utils/VTL_time.c
    VTL/utils/VTL_file.c
    VTL/utils/VTL_encryptor.c

    VTL/utils/log/VTL_console_log.c

    VTL/utils/db/VTL_db_credentals.c

    )

set(VTL_CMAKE_USER_SRC_FILES

    VTL/user/VTL_user_data.c
    VTL/user/history/VTL_user_history_data.c
    VTL/user/history/db/VTL_user_history_save.c    

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

set(VTL_CMAKE_MEDIA_CONTAINER_IMG_SRC_FILES 
    VTL/media_container/img/VTL_img_core.c
    VTL/media_container/img/VTL_img_filters.c
    VTL/media_container/img/VTL_img_utils.c
)

set(VTL_CMAKE_MEDIA_CONTAINER_SRC_FILES

    ${VTL_CMAKE_MEDIA_CONTAINER_AUDIO_SRC_FILES}
    ${VTL_CMAKE_MEDIA_CONTAINER_VIDEO_SRC_FILES}
    ${VTL_CMAKE_MEDIA_CONTAINER_IMG_SRC_FILES}

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

add_subdirectory(VTL/media_container/img)

add_executable(${AppName}
    ${VTL_CMAKE_UTILS_SRC_FILES}
    ${VTL_CMAKE_USER_SRC_FILES}
    ${VTL_CMAKE_PUBLICATION_SRC_FILES}
    ${VTL_CMAKE_MEDIA_CONTAINER_SRC_FILES}
    ${VTL_CMAKE_CONTENT_PLATFORM_SRC_FILES}
    main.c)

# Добавлено для поддержки libass через pkg-config (ASS/SSA субтитры)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBASS REQUIRED libass)

# Link FFmpeg
target_link_libraries(${AppName} PRIVATE 
    ffmpeg 
    VTL_img 
    m           # математическая библиотека
    ${LIBASS_LIBRARIES} ${LIBASS_LDFLAGS} # корректная линковка libass через pkg-config
#    va          # VAAPI (только для Linux, на macOS не поддерживается)
#    va-drm      # VAAPI DRM (только для Linux, на macOS не поддерживается)
#    va-x11      # VAAPI X11 (только для Linux, на macOS не поддерживается)
#    drm         # Direct Rendering Manager (только для Linux, на macOS не поддерживается)
#    X11         # X11 (только для Linux/Unix, на macOS не поддерживается)
#    vdpau       # VDPAU (только для Linux, на macOS не поддерживается)
    z           # zlib
    bz2         # bzip2
    lzma        # xz/lzma
#    lz4         # lz4 (только для Linux, на macOS не поддерживается)
#    zstd        # zstd (только для Linux, на macOS не поддерживается)
#    x264        # H.264 (только для Linux, на macOS не поддерживается)
#    x265        # H.265 (только для Linux, на macOS не поддерживается)
#    vpx         # VP8/VP9 (только для Linux, на macOS не поддерживается)
#    aom         # AV1 (только для Linux, на macOS не поддерживается)
#    opus        # Opus audio (только для Linux, на macOS не поддерживается)
#    vorbis      # Vorbis audio (только для Linux, на macOS не поддерживается)
#    ogg         # Ogg container (только для Linux, на macOS не поддерживается)
#    swresample  # FFmpeg resampling (линковка не требуется на macOS, всё тянет ffmpeg)
#    swscale     # FFmpeg scaling (линковка не требуется на macOS, всё тянет ffmpeg)
    "-framework OpenGL"               # Для CGLGetCurrentContext (OpenGL контекст, используется CoreImage)
    "-framework Metal"                # Для MTLCreateSystemDefaultDevice (Metal API, используется FFmpeg фильтрами)
)

target_include_directories(${AppName} 
    PRIVATE 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/VTL
    ${CMAKE_SOURCE_DIR}/VTL/user
    ${CMAKE_SOURCE_DIR}/VTL/user/history
    ${FFMPEG_INCLUDE_DIRS}
    SYSTEM
    /usr/include
    ${LIBASS_INCLUDE_DIRS}
)

enable_testing()

add_subdirectory(VTL/test)

# Следующие фреймворки нужны для поддержки мультимедиа и CoreFoundation на macOS
if(APPLE)
    target_link_libraries(${AppName} PRIVATE
        "-framework CoreFoundation"      # Для работы с CoreFoundation (CFArray, CFData и др.)
        "-framework CoreMedia"           # Для работы с CoreMedia (CMSampleBuffer и др.)
        "-framework VideoToolbox"        # Для аппаратного видеокодирования/декодирования
        "-framework AVFoundation"        # Для мультимедиа (AVAsset, AVPlayer и др.)
        "-framework CoreGraphics"        # Для работы с изображениями (CGImage, CGContext и др.)
        "-framework CoreVideo"           # Для работы с видеобуферами (CVPixelBuffer и др.)
        "-framework AudioToolbox"        # Для работы с аудио (AudioConverter и др.)
        "-framework Foundation"          # Для Objective-C классов (NSString, NSArray и др.)
        "-framework AppKit"              # Для CoreImage фильтров (NSOpenGLPixelFormat и др.)
        "-framework CoreImage"           # Для фильтров и работы с изображениями (CIImage, CIFilter и др.)
        "-framework Security"            # Для SSL/TLS (SSLClose, SSLCreateContext и др.)
        iconv                             # Для поддержки iconv (конвертация кодировок)
    )
endif()