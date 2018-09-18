//
// Created by goforbroke on 18.09.18.
//

#ifndef GFB_FIND_THREADS_H
#define GFB_FIND_THREADS_H

#include <iostream>
#include <mutex>
#include <condition_variable>

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

    /*Semaphore(const Semaphore &other) noexcept {
        cout << "semaphore copy" << endl;
        this->count = other.count;
    }

    Semaphore(const Semaphore &&other) noexcept {
        cout << "semaphore move" << endl;
        this->count = other.count;
    }*/

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

#endif //GFB_FIND_THREADS_H
