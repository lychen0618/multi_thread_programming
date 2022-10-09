#pragma once
#include <condition_variable>
#include <mutex>

class RWLock {
private:
    std::mutex m;
    // 如果是正数，说明有ref_count个数个读线程在临界区；
    // 如果为-1，说明有一个写线程在临界区
    int ref_count = 0;
    int reader_num = 0;
    int writer_num = 0;
    std::condition_variable reader_cond, writer_cond;

public:
    RWLock() {}
    ~RWLock() = default;

    bool get_rd_lock() {
        std::unique_lock<std::mutex> lock(m);
        while (ref_count < 0 || writer_num > 0) {
            ++reader_num;
            reader_cond.wait(lock);
            --reader_num;
        }
        ++ref_count;
    }

    bool get_wt_lock() {
        std::unique_lock<std::mutex> lock(m);
        while (ref_count != 0) {
            ++writer_num;
            writer_cond.wait(lock);
            --writer_num;
        }
        ref_count = -1;
    }

    bool unlock() {
        std::unique_lock<std::mutex> lock(m);
        if (ref_count > 0)
            --ref_count;
        else
            ref_count = 0;
        if (writer_num == 0 && reader_num > 0)
            reader_cond.notify_all();
        else if (writer_num != 0 && ref_count == 0)
            writer_cond.notify_one();
    }
};