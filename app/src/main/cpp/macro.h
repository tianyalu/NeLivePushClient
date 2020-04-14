//
// Created by tian on 2020/4/15.
//

#ifndef NELIVEPUSHCLIENT_MACRO_H
#define NELIVEPUSHCLIENT_MACRO_H

#include <android/log.h>

#define THREAD_MAIN 1
#define THREAD_CHILD 2

#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR, "sty", FORMAT, ## __VA_ARGS__);
#define LOGE2(...) __android_log_print(ANDROID_LOG_ERROR, "NEPLAYER_NATIVE",__VA_ARGS__)

#define DELETE(object) if (object) { delete object; object = 0; }

#endif //NELIVEPUSHCLIENT_MACRO_H
