cmake_minimum_required(VERSION 3.4.1)
#引用指定目录下的CMakeLists.txt
add_subdirectory(librtmp)
#指定头文件查找路径
include_directories(librtmp)
include_directories(x264/include)

#这里只能用CMAKE_CXX_FLAGS
#指定库的查找路径
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/x264/libs/${CMAKE_ANDROID_ARCH_ABI}")
add_library( # Sets the name of the library.
             native-lib
             # Sets the library as a shared library.
             SHARED
             # Provides a relative path to your source file(s).
             native-lib.cpp )

target_link_libraries( # Specifies the target library.
                       native-lib
                       rtmp
                       x264
                       log )