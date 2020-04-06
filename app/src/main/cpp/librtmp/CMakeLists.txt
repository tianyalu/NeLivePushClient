cmake_minimum_required(VERSION 3.4.1)
#预编译宏: -D 定义宏
#纯C的第三方库就用CMAKE_C_FLAGS，否则只要有一个cpp文件就需要用CMAKE_CXX_FLAGS
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_CRYPTO")
#所有源文件放入`rtmp_srcs`变量
file(GLOB rtmp_srcs *.c)
#编译静态库
add_library( # Sets the name of the library.
             rtmp
             # Sets the library as a shared library.
             STATIC
             # Provides a relative path to your source file(s).
             ${rtmp_srcs}
        )
