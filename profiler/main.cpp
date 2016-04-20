#include "profiler.h"

#include <inttypes.h>
#include <stdint.h>

#include <cstdio>
#include <thread>

using namespace art_profiler;

void f() {
    int64_t sum = 0;
    int j = 0;
    for (int i = 0; i < 100000000; ++i) {
        if (++j % 37 == 0) continue;
        sum += i;
    }
    printf("sum = %" PRId64 " \n", sum);
}

void f0() {
    Prof prof(0);
    f();
}

void f1() {
    Prof prof(1);
    f();
}

void f2() {
    Prof prof(2);
    f();
}

void SingleThreadTest() {
    using namespace art_profiler;
    ProfMgr mgr(ProfMgr::SINGLE_THREAD, 1);
    mgr.add_tag_name(0, "TEST");
    {
        Prof prof("foo");
        f();
    }
    {
        Prof prof("foo");
        f();
    }
    mgr.Report();
}

void MultiThreadTest() {
    ProfMgr mgr(ProfMgr::MULTI_THREAD, 3);
    mgr.add_tag_name(0, "A");
    mgr.add_tag_name(1, "AA");
    mgr.add_tag_name(2, "AAA");

    std::thread t0 {f0};
    std::thread t1 {f1};
    std::thread t01 {f0};

    t0.join();
    t1.join();
    t01.join();

    mgr.Report();
}

int main() {
    // SingleThreadTest()
    MultiThreadTest();

    return 0;
}
