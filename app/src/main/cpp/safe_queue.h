//
// Created by tian on 2020/4/14.
//

#ifndef NELIVEPUSHCLIENT_SAFE_QUEUE_H
#define NELIVEPUSHCLIENT_SAFE_QUEUE_H

#include <queue>
#include <pthread.h>

using namespace std;

template<typename T>
class SafeQueue {
    //本来是定义一个函数指针：ReleaseCallback，这个指针指向参数为T*，返回值为void的函数
    //加上typedef后，ReleaseCallback表示一种类型，这种类型可以定义 指向参数为T*，返回值为void的函数指针
    typedef void(*ReleaseCallback) (T *); //定义一种类型（ReleaseCallback），这种类型可以定义函数指针
    typedef void(*SyncCallback) (queue<T> &);

public:
    SafeQueue() {
        pthread_mutex_init(&mutex, 0); //动态初始化
        pthread_cond_init(&cond, 0);
    }

    ~SafeQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    /**
     * 入队
     * @param value
     */
    void push(T value) {
        pthread_mutex_lock(&mutex);
        if(work) {
            q.push(value);
            pthread_cond_signal(&cond); //发送通知，队列有数据了
        }else {
            //需要释放？
            //交给调用者释放
            if(releaseCallback) {
                releaseCallback(&value);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    int pop(T &value) {
        int ret = 0;
        pthread_mutex_lock(&mutex);
        while (work && q.empty()) {
            //工作状态并且队列为空，一直等待
            pthread_cond_wait(&cond, &mutex);
        }
        //TODO 不工作但时如果队列中有数据还可以出队列，这里可能有坑
        if(!q.empty()) {
            value = q.front();
            q.pop();
            ret = 1;
        }
        pthread_mutex_unlock(&mutex);
        return ret;
    }

    int empty() {
        return q.empty();
    }

    int size() {
        return q.size();
    }

    void clear() {
        pthread_mutex_lock(&mutex);
        unsigned int size = q.size();
        for (int i = 0; i < size; ++i) {
            T value = q.front();
            //释放
            if(releaseCallback) {
                releaseCallback(&value);
            }
            q.pop();
        }
        pthread_mutex_unlock(&mutex);
    }

    void setWork(int work) {
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    void sync() {
        pthread_mutex_lock(&mutex);
        syncCallback(q);
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallback(ReleaseCallback releaseCallback) {
        this->releaseCallback = releaseCallback;
    }

    void setSyncCallback(SyncCallback syncCallback) {
        this->syncCallback = syncCallback;
    }

private:
    queue<T> q;
    pthread_mutex_t mutex; //互斥锁
    pthread_cond_t cond; //条件变量
    int work; //标记队列是否工作状态
    ReleaseCallback releaseCallback;
    SyncCallback syncCallback;
};

#endif //NELIVEPUSHCLIENT_SAFE_QUEUE_H
