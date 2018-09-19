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
    mutex buffer_mutex;
    mutex wait_not_empty_mutex;
    condition_variable cv;
public:
    explicit Channel(size_t c) : capacity(c) {}

    void operator<<(const T obj) {
//        cout << "-- wait for writing" << endl;
        unique_lock<mutex> lock(buffer_mutex);
//        cout << "-- wait for free space" << endl;
        cv.wait(lock, [this]() { return buffer.size() < capacity; });
//        cout << "-- writing" << endl;
        buffer.push(obj);
    }

    Channel<T> &operator>>(T &obj) {
//        cout << "-- wait not empty" << endl;
//        unique_lock<mutex> lock1(wait_not_empty_mutex);
//        cv.wait(lock1, [this]() { return buffer.size() > 0; });
//        cout << "-- not empty now" << endl;
//        unique_lock<mutex> lock2(buffer_mutex);
//        obj = buffer.front();
//        cout << "-- read front" << endl;
//        buffer.pop();
//        return *this;
        while (true) {
            if (buffer.size() > 0) {
                unique_lock<mutex> lock2(buffer_mutex);
                obj = buffer.front();
                buffer.pop();
                break;
            }
        }
        return *this;
    }
};

#endif //GFB_FIND_THREADS_H
