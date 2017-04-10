# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

set(lib_src_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})
include_directories(
     ${CMAKE_SOURCE_DIR}/src/main/cpp/include
)
add_library(sdl2_lib SHARED IMPORTED)
set_target_properties(sdl2_lib PROPERTIES IMPORTED_LOCATION
       ${lib_src_DIR}/libSDL2.so)

file(GLOB SOURCE_MAIN "${CMAKE_SOURCE_DIR}/src/main/cpp/*.c")

add_library( main  SHARED ${SOURCE_MAIN})

#Add other prebuilt libraries
              add_library(avcodec-lib  SHARED IMPORTED )
              set_target_properties(avcodec-lib PROPERTIES IMPORTED_LOCATION
                                    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/armeabi/libavcodec-57.so )
              add_library(avdevice-lib  SHARED IMPORTED )
              set_target_properties(avdevice-lib PROPERTIES IMPORTED_LOCATION
                                    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/armeabi/libavdevice-57.so )
              add_library(avfilter-lib  SHARED IMPORTED )
              set_target_properties(avfilter-lib PROPERTIES IMPORTED_LOCATION
                                    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/armeabi/libavfilter-6.so )
              add_library(avformat-lib  SHARED IMPORTED )
              set_target_properties(avformat-lib PROPERTIES IMPORTED_LOCATION
                                    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/armeabi/libavformat-57.so )
              add_library(avutil-lib  SHARED IMPORTED )
              set_target_properties(avutil-lib PROPERTIES IMPORTED_LOCATION
                                    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/armeabi/libavutil-55.so )
              add_library(swresample-lib  SHARED IMPORTED )
              set_target_properties(swresample-lib PROPERTIES IMPORTED_LOCATION
                                    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/armeabi/libswresample-2.so )
              add_library(swscale-lib  SHARED IMPORTED )
              set_target_properties(swscale-lib PROPERTIES IMPORTED_LOCATION
                                    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/armeabi/libswscale-4.so )


             # Specifies a path to native header files.
            # include_directories(src/main/cpp/include/)
             include_directories(src/main/cpp/FFmpeg/include/)

include_directories(
            ${CMAKE_SOURCE_DIR}/src/main/cpp/include )
            # include_directories(src/main/cpp/include/)
             include_directories(src/main/cpp/FFmpeg/include/)
find_library( # Sets the name of the path variable.
              log-lib
              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )
 find_library(GLESv1_CM-lib    GLESv1_CM )
  find_library(GLESv2-lib    GLESv2 )

target_link_libraries( # Specifies the target library.
                      main
                      ${GLESv1_CM-lib}
                       ${GLESv2-lib}
                       sdl2_lib
                       avcodec-lib
                       avdevice-lib
                       avfilter-lib
                        avformat-lib
                        avutil-lib
                        swresample-lib
                        swscale-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )
