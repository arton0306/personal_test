#include "profiler.h"

#include <inttypes.h>
#include <stdint.h>

#include <cstdio>

void foo() {
    int64_t sum = 0;
    int j = 0;
    for (int i = 0; i < 100000000; ++i) {
        if (++j % 37 == 0) continue;
        sum += i;
    }
    printf("sum = %" PRId64 " \n", sum);
}

int main() {
    ProfMgr mgr(ProfMgr::SINGLE_THREAD, 1);
    mgr.add_tag_name(0, "TEST");
    {
        Prof prof("foo");
        foo();
    }
    {
        Prof prof("foo");
        foo();
    }
    mgr.Report();

    return 0;
}
