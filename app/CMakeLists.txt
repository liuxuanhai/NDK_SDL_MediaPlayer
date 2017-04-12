# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

project(MediaPlayer)

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

set(requirLibs SDL2 avcodec-57 avdevice-57 avfilter-6 avformat-57 avutil-55 swresample-2 swscale-4)
foreach(lib ${requirLibs})
	add_library(${lib} SHARED IMPORTED)
endforeach(lib)

include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/FFmpeg/include/ )
include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/include )
include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/FFmpeg/include/ )

file(GLOB SOURCE_MAIN "${CMAKE_SOURCE_DIR}/src/main/cpp/*.c")

#For Android NDK.
if(ANDROID_NDK)
	message("PLATFORM: Android")
	set(libDir ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi )
	
	foreach(lib ${requirLibs})
		set_target_properties(${lib} PROPERTIES IMPORTED_LOCATION ${libDir}/lib${lib}.so )
	endforeach(lib)
	#重新定位SDL2的动态链接库
	set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libSDL2.so )
	
	add_library(main  SHARED ${SOURCE_MAIN} )
	
	find_library(log-lib log )
	find_library(GLESv1_CM-lib GLESv1_CM )
	find_library(GLESv2-lib GLESv2 )
	target_link_libraries(
		main
		${requirLibs}
		${log-lib}
		${GLESv1_CM-lib}
		${GLESv2-lib} )
		
#For Windows(VisualStudio)
elseif(WIN32)
	if(CMAKE_CL_64)
		SET(PLATFORM x64)
	else(CMAKE_CL_64)
		SET(PLATFORM x86)
	endif(CMAKE_CL_64)
	message("PLATFORM: Windows ${PLATFORM}" )
	
    set(libDir ${CMAKE_SOURCE_DIR}/src/main/winLibs/${PLATFORM} )
	
	
	foreach(lib ${requirLibs})
		set_target_properties(${lib} PROPERTIES IMPORTED_LOCATION ${libDir}/bin/${lib}.dll )
		set_property(TARGET ${lib} PROPERTY IMPORTED_IMPLIB ${libDir}/${lib}.lib )
	endforeach(lib)
	
	#额外链接一个SDL2main.lib
	add_library(SDL2main SHARED IMPORTED)
	set_property(TARGET SDL2main PROPERTY IMPORTED_IMPLIB ${libDir}/SDL2main.lib )
	
	#消除链接时的安全检查
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO" )
	add_executable(main ${SOURCE_MAIN} )
	#需要自己在vs中把工作目录设置到${libDir}\bin下，不然运行时找不到*.dll
	include_directories(${OPENGL_INCLUDE_DIR} )
	
	target_link_libraries(
		main
		SDL2main
		${requirLibs}
		${OPENGL_gl_LIBRARY} )
endif()