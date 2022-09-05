#pragma once
#include <atomic>

class SpinLock {
private:
    std::atomic_flag flag;

public:
    SpinLock() : flag(ATOMIC_FLAG_INIT) {}

    void lock() {
        // Atomically changes the state of a std::atomic_flag to set (true) and
        // returns the value it held before.
        while (flag.test_and_set())
            ;
    }

    void unlock() { flag.clear(); }
};