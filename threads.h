//
// Created by goforbroke on 18.09.18.
//

#ifndef GFB_FIND_THREADS_H
#define GFB_FIND_THREADS_H

#include <iostream>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <queue>

using namespace std;

class Semaphore {
private:
    unsigned int count = 0; // left free sections count
    mutex count_mutex;
    condition_variable cv;
public:
    Semaphore(unsigned int count) noexcept {
        this->count = count;
    }

    void wait() { // take section
        unique_lock<mutex> lock(count_mutex);
        cv.wait(lock, [this]() { return count > 0; });
        count--;
        cout << "start new thread" << endl;
    }

    void notify() { // free section
        unique_lock<mutex> lock(count_mutex);
        count++;
        cout << "end new thread" << endl;
        cv.notify_one();
    }
};

class CriticalSection {
private:
    Semaphore &semaphore;
public:
    CriticalSection(Semaphore &s) : semaphore(s) { semaphore.wait(); }

    ~CriticalSection() { semaphore.notify(); }
};

template<class T>
class Channel {
private:
    std::queue<T> buffer;
    size_t capacity;
    mutex capacity_mutex;
    condition_variable cv;
public:
    Channel(size_t c) : capacity(c) {}

    void operator<<(const T obj) {
        unique_lock<mutex> lock(capacity_mutex);
        cv.wait(lock, [this]() { return buffer.size() < capacity; });
        buffer.push(obj);
    }

    T operator>>(T &obj) {
        unique_lock<mutex> lock(capacity_mutex);
        const T r = buffer.front();
        buffer.pop();
        return r;
    }
};

#endif //GFB_FIND_THREADS_H
