#include "profiler.h"

#include <inttypes.h>
#include <stdint.h>
#include <time.h>

#include <cstdio>
#include <iostream>
#include <map>
#include <set>

using namespace std;

namespace {

string Ns2Str(int64_t ns)
{
    int sec = static_cast<int>(ns / 1000000000LL);
    int ss = sec % 60;
    int mm = sec / 60 % 60;
    int hh = sec / 60 / 60;
    int ms = ns / 1000000 % 1000;

    char buf[64];
    snprintf(buf, 64, "%02d:%02d:%02d.%03d", hh, mm, ss, ms);

    return buf;
}

} // namespace

namespace art_profiler {

ProfMgr *Prof::sProfMgr_ = NULL;

Prof::Prof(int tag)
{
    tag_ = tag;
    title_ = NULL;
    clock_gettime(CLOCK_REALTIME, &start_);
    assert(sProfMgr_);
    assert(tag < sProfMgr_->get_tag_count());
}

Prof::Prof(char const *title)
{
    tag_ = 0;
    title_ = title;
    clock_gettime(CLOCK_REALTIME, &start_);
}

Prof::~Prof()
{
    clock_gettime(CLOCK_REALTIME, &end_);
    if (sProfMgr_) {
        sProfMgr_->AddProf(this);
    }

    if (sProfMgr_ && title_) {
        int64_t begin_ns = get_ns(start_) - get_ns(sProfMgr_->get_mgr_init_time());
        printf("%25s ] begin at: %s  period: %s \n",
                title_,
                Ns2Str(begin_ns).c_str(),
                Ns2Str(get_period_ns()).c_str());
    } else {
        printf("tag:%02d ] period: %s \n",
                tag_,
                Ns2Str(get_period_ns()).c_str());
    }
}

ProfMgr::ProfMgr(Mode mode, int tag_count, int (*thrd_id_cb)(void))
    : mode_(mode)
    , tag_count_(tag_count)
    , thread_id_callback_(thrd_id_cb)
    , tag_periods_(tag_count, vector<int64_t>(kPrime, 0))
    , tag_names_(tag_count)
{
    if (mode == SINGLE_THREAD) assert(!thrd_id_cb);
    assert(tag_count >= 1);
    clock_gettime(CLOCK_REALTIME, &mgr_init_time_);
    Prof::set_prof_mgr(this);
}

void ProfMgr::AddProf(Prof const *prof)
{
    int slot = -1;
    if (mode_ == SINGLE_THREAD) {
        slot = 0;
    } else if (thread_id_callback_) {
        slot = thread_id_callback_();
    } else {
        Timespec tmp;
        clock_gettime(CLOCK_REALTIME, &tmp);
        slot = tmp.tv_nsec % kPrime; // pseudo random
    }
    assert(-1 < slot && slot < kPrime);
    if (0) { // debug
        printf("(debug) %s %" PRId64 " %" PRId64 " \n",
                prof->get_title(),
                prof->get_start_ns(),
                prof->get_end_ns());
    }
    tag_periods_[prof->get_tag()][slot] += prof->get_period_ns();
}

void ProfMgr::PrintTagTotalNs(int tag, int64_t total_ns) const
{
    if (tag_names_[tag] != "") {
        printf("%10s  total: %s\n",
                tag_names_[tag].c_str(),
                Ns2Str(total_ns).c_str());
    } else {
        printf("tag-%02d  total: %s\n",
                tag,
                Ns2Str(total_ns).c_str());
    }
}

void ProfMgr::Report() const
{
    cout << "========== profile report ==========" << endl;

    for (int i = 0; i < tag_count_; ++i) {
        int64_t total_ns = 0;
        for (int j = 0; j < kPrime; ++j) {
            total_ns += tag_periods_[i][j];
        }
        PrintTagTotalNs(i, total_ns);
    }

    /*
    // merge
    vector< map<string, int64_t> > tbl(kPrime, map<string, int64_t>());
    set<string> titles;
    for (int i = 0; i < kPrime; ++i) {
        for (int j = 0; j < tag_periods_[i].size(); ++j) {
            InnerProf &iprof = tag_periods_[i][j];
            if (titles.count(iprof.title) == 0) {
                titles.insert(iprof.title);
                tbl[i][iprof.title] = get_ns(iprof.end) - get_ns(iprof.start);
            } else {
                tbl[i][iprof.title] += get_ns(iprof.end) - get_ns(iprof.start);
            }
        }
    }
    */
}

} // namespace art_profiler
