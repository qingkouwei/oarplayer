# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_library( # Sets the name of the library.
             srslibrtmp-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/srs_librtmp.cpp )

add_library( # Sets the name of the library.
             oarp-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/oarplayer_jni.c
             src/main/cpp/jni_utils.c
             src/main/cpp/srs_readthread.c
             src/main/cpp/srs_librtmp.cpp
             src/main/cpp/oar_clock.c
             src/main/cpp/oar_packet_queue.c
             src/main/cpp/oar_frame_queue.c
             src/main/cpp/oar_player.c
             src/main/cpp/oar_jni_reflect.c
             src/main/cpp/oar_mediacodec.c
             src/main/cpp/oar_player_video_hw_decode_thread.c
             src/main/cpp/oar_video_render.c
             src/main/cpp/oar_texture.c
             src/main/cpp/oar_glsl_program.c
             src/main/cpp/oar_player_gl_thread.c
             )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )
find_library(android-lib
            android)
find_library(glesv2-lib GLESv2)
find_library(egl-lib EGL)
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
target_link_libraries( # Specifies the target library.
                       srslibrtmp-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       )
if (${CMAKE_SYSTEM_VERSION} MATCHES "21")
    find_library(media-ndk mediandk)
    target_link_libraries( # Specifies the target library.
                       oarp-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       ${android-lib}
                       ${egl-lib}
                       ${glesv2-lib}
                       ${media-ndk}
                       #${srslibrtmp-lib}
                       )
else (${CMAKE_SYSTEM_VERSION} LESS "21")
    target_link_libraries( # Specifies the target library.
                           oarp-lib

                           # Links the target library to the log library
                           # included in the NDK.
                           ${log-lib}
                           ${android-lib}
                           ${egl-lib}
                           ${glesv2-lib}
                           #${srslibrtmp-lib}
                           )
endif(${CMAKE_SYSTEM_VERSION} MATCHES "21")