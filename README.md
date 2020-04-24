# NeLivePushClient  直播推流-客户端准备工作

[TOC]  

## 一、`librtmp`编译集成

参考：[https://www.jianshu.com/p/55ffaf8ba0ab](https://www.jianshu.com/p/55ffaf8ba0ab)  

### 1.1 简介

[RTMPDump](http://rtmpdump.mplayerhq.hu/) 是一个用来处理`RTMP`流媒体的开源工具包，它能够单独使用进行`RTMP`的通信，也可以集成到FFmpeg中通过`FFmpeg`接口来使用`RTMPDump`。  

在Android中可以直接借助`NDK`在`JNI`层调用`RTMPDump`来完成`RTMP`通信。  

> 在根目录提供了一个`Makefile`与一些`.c`源文件，这里的源文件将会编译出一系列的可执行文件。然后我们需要的并不是可执行文件，真正的对`RTMP`的实现都在`librtmp`子目录中。 在这个子目录中同样包含了一个`Makefile`文件，通过阅读`Makefile`发现，它的源码并不多：`OBJS=rtmp.o log.o amf.o hashswf.o parseurl.o`。 因此我们不进行预编译，即直接放入AS中借助`CMakeLists.txt`来进行编译。这么做可以让我们方便地对库本身进行调试或修改（实际上我们确实会稍微修改这个库的源码）。  

### 1.2 编译集成步骤

#### 1.2.1 下载源码
下载地址：[http://rtmpdump.mplayerhq.hu/download](http://rtmpdump.mplayerhq.hu/download)  

这里我们下载的版本是 [rtmpdump-2.3.tgz](http://rtmpdump.mplayerhq.hu/download/rtmpdump-2.3.tgz)  

#### 1.2.2 复制`librtmp`并编写`CMakeLists.txt`

在AS中新建项目，复制`librtmp`到`src/main/cpp/librtmp`，并在该目录下为其编写`CMakeLists.txt`：  

```cmake
cmake_minimum_required(VERSION 3.4.1)
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
```

#### 1.2.3 在`app/CMakeLists.txt`中导入这个`CMakeLists.txt`  

```cmake
cmake_minimum_required(VERSION 3.4.1)
#引用指定目录下的CMakeLists.txt
add_subdirectory(librtmp) 
#指定头文件查找路径
include_directories(librtmp) 

add_library( # Sets the name of the library.
             native-lib
             # Sets the library as a shared library.
             SHARED
             # Provides a relative path to your source file(s).
             native-lib.cpp )

target_link_libraries( # Specifies the target library.
                       native-lib
                       rtmp
                       log )
```
#### 1.2.4 编译测试

编译测试输出版本信息，修改`native-lib.cpp`文件：  

```c++
#include <jni.h>
#include <string>
#include <rtmp.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_sty_ne_livepushclient_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    char version[100];
    sprintf(version, "rtmp version : %d", RTMP_LibVersion());
    return env->NewStringUTF(version);
}
```

编译报错如下：  

```bash 
E:\xxx\xxx\NeLivePushClient\app\src\main\cpp\librtmp\hashswf.c:56:10: fatal error: 'openssl/ssl.h' file not found
```

问题排查：  

打开`rtmp.c`，我们发现这里有一段宏定义：  

```c
#ifdef CRYPTO
#ifdef USE_POLARSSL
#include <polarssl/havege.h>
#elif defined(USE_GNUTLS)
#include <gnutls/gnutls.h>
#else	/* USE_OPENSSL */
#include <openssl/ssl.h>
#include <openssl/rc4.h>
#endif
TLS_CTX RTMP_TLS_ctx;
#endif
```

最终只有`CRYPTO`这个宏被定义了才会`#include <openssl/ssl.h>`，那么我们继续查找`CRYPTO`定义的地方，在`rtmp.h`中又有这样一段：  

```C 
#if !defined(NO_CRYPTO) && !defined(CRYPTO)
#define CRYPTO
#endif
```

我们只需要编译时添加定义`NO_CRYPTO`这个预编译宏就可以了。修改`librtmp`中的`CMakeLists.txt`：  

```cmake
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
```

重新编译运行，输出`rtmp version: 131840`，`131840`对应的16进制为：`0x020300`, 也就是2.3版本。  

## 二、`x264`交叉编译

参考：[https://www.jianshu.com/p/a99b518f29b9](https://www.jianshu.com/p/a99b518f29b9)  

### 2.1 简介

[x264](https://www.videolan.org/developers/x264.html) 是一个`C`语言编写的目前对`H.264`标准支持最完善的编解码库。它与`RTMPDump`一样，可以在`Android`中直接使用，也可以集成进入`FFMpeg`。

### 2.2 准备工作

需要先下载并配置`Android NDK`环境，若已经下载过，可忽略此步骤。  

#### 2.2.1 下载`NDK r17`版本

下载地址：[https://developer.android.google.cn/ndk/downloads/older_releases.html](https://developer.android.google.cn/ndk/downloads/older_releases.html)  

可以在`Windows`环境下下载，然后通过`xshell` 的`xftp` 上传到服务器的`Linux`环境。

#### 2.2.2 解压

```bash
unzip android-ndk-r17c-linux-x86_64.zip
```

#### 2.2.3 进入`NDK`解压目录，查看当前路径

```bash
[root@iZwz9ci7skvj0jj2sfdmqgZ software]# cd android-ndk-r17c/
[root@iZwz9ci7skvj0jj2sfdmqgZ android-ndk-r17c]# pwd
/root/software/android-ndk-r17c
```

#### 2.2.4 修改配置文件

```bash
vim /etc/profile # 若没有写权限，请先添加权限
```

#### 2.2.5 添加`NDK`路径到配置文件中

```bash
NDKROOT=/root/software/android-ndk-r17c
export PATH=$NDKROOT:$PATH  # 添加到环境变量中
```

#### 2.2.6 使配置生效

```bash
source /etc/profile
```

#### 2.2.7 验证是否配置成功

```bash
[root@iZwz9ci7skvj0jj2sfdmqgZ android-ndk-r17c]# echo $NDKROOT
/root/software/android-ndk-r17c
```

### 2.3 交叉编译步骤

#### 2.3.1 下载

在Linux环境下下载  

```bash 
wget https://code.videolan.org/videolan/x264/-/archive/master/x264-master.tar.bz2
```

#### 2.3.2 解压

```bash
tar -xvf x264-master.tar.bz2
```

#### 2.3.3 进入`x264`目录并编写编译脚本

```bash
cd x264-master/
vim build_x264.sh
```
> 可以通过查看`configure`文件命令来查看我们可以配置的参数：  `./configure --help`

我们要编写的`build_x264.sh`脚本内容如下：

```bash
#!/bin/bash

NDK_ROOT=/root/software/android-ndk-r17c

PREFIX=./output/livepush/armeabi-v7a

TOOLCHAIN=$NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64

CFLAGS="-isysroot $NDK_ROOT/sysroot -isystem $NDK_ROOT/sysroot/usr/include/arm-linux-androideabi -D__ANDROID_API__=17 -g -DANDROID -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -mthumb -Wa,--noexecstack -Wformat -Werror=format-security  -O0 -fPIC"

# --disable-cli : 关闭命令行
# 其它和FFmpeg一样
./configure \
--prefix=$PREFIX \
--disable-cli \
--enable-static \
--enable-pic \
--host=arm-linux \
--cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
--sysroot=$NDK_ROOT/platforms/android-17/arch-arm \
--extra-cflags="$CFLAGS"

make clean
make install
```

#### 2.3.4 编译

```bash
chmod +x build_x264.sh
./build_x264.sh
```

#### 2.3.5 将编译产物打包并传给`Windows`

```bash
[root@iZwz9ci7skvj0jj2sfdmqgZ x264-master]# cd ./output/livepush/
[root@iZwz9ci7skvj0jj2sfdmqgZ livepush]# ls
armeabi-v7a
[root@iZwz9ci7skvj0jj2sfdmqgZ livepush]# zip -r x264.zip *
[root@iZwz9ci7skvj0jj2sfdmqgZ livepush]# ls
armeabi-v7a  x264.zip
```

同样可以借助`xshell` 的`xftp` 从服务器下载`x264.zip`到`Windows`环境。  

### 2.4 `AS`中集成`x264`

#### 2.4.1 拷贝`2.3.5`下载的文件到`AS`中  

目录结构如下图所示：  

![image](https://github.com/tianyalu/NeLivePushClient/raw/master/show/x264_dir_structure.png)  

修改`app/src/main/cpp`根目录下的`CMakeLists.txt`文件：  

```cmake
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
```

#### 2.4.2 测试

修改`app/src/main/cpp`目录下的`native-lib.cpp`文件：  

```c++
#include <jni.h>
#include <string>
#include <rtmp.h>
#include <x264.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_sty_ne_livepushclient_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    char version[100];
    sprintf(version, "rtmp version : %d", RTMP_LibVersion());
    //测试x264
    x264_picture_t *pic = new x264_picture_t;
    x264_picture_init(pic);
    return env->NewStringUTF(version);
}
```

注意要在`app/build.gradle`文件中添加`abiFilters`：  

```groovy
android {
    compileSdkVersion 28
    buildToolsVersion "29.0.2"

    defaultConfig {
        applicationId "com.sty.ne.livepushclient"
        minSdkVersion 21
        targetSdkVersion 28
        versionCode 1
        versionName "1.0"

        testInstrumentationRunner "androidx.test.runner.AndroidJUnitRunner"

        externalNativeBuild {
            cmake {
                cppFlags ""
                abiFilters "armeabi-v7a"
            }
        }
        ndk {
            abiFilters "armeabi-v7a"
        }
    }

    buildTypes {
        release {
            minifyEnabled false
            proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
        }
    }

    externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
            version "3.10.2"
        }
    }
}
```

编译运行成功代码集成成功。  

## 三、`RTMP`视频数据格式
参考：[`RTMP`视频数据格式](https://www.jianshu.com/p/0c882eca979c)  

## 四、`FLV`文件格式
参考：[`FLV`文件格式](https://www.jianshu.com/p/4f6ef65e8b97)  

## 五、扩展参考
[音视频基础知识](https://www.jianshu.com/p/a2c09daee428)
[音视频基础知识-图像篇](https://www.jianshu.com/p/0f0ff3d2f5d4)  

## 六、音频推流
视频频推流代码完成后，可以用在浏览器输入服务器地址查看效果，如下图所示：  
![image](https://github.com/tianyalu/NeLivePushClient/raw/master/show/push_success.png)  
可以在用`ffplay`命令预览直播推流效果：  
```bash
ffplay -i rtmp://47.115.6.127/myapp/
```

