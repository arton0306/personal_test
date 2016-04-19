#ifndef ARTONINC_PROFILE_H_
#define ARTONINC_PROFILE_H_

#include <stdint.h>

#include <cassert>
#include <string>
#include <vector>

namespace art_profiler {

typedef struct timespec Timespec;
void PrintTime(int64_t ns, char const *title);
inline int64_t get_ns(Timespec const &s) { return s.tv_sec * 1000000000LL + s.tv_nsec; }

// ProfMgr is for profiling.
// Creates one ProfMgr, and creates Prof, and finally calls Report.
// Example 1:
//   // The running time of the same tag will be added together and report as one.
//   enum {
//       TAG1,
//       TAG2,
//       TAG_COUNT
//   }
//   ProfMgr mgr(ProfMgr::SINGLE_THREAD, TAG_COUNT);
//   // or ProfMgr mgr(ProfMgr::MULTI_THREAD, TAG_COUNT);
//   mgr.AddTagName(TAG1, "module1"); // optional
//   mgr.AddTagName(TAG2, "module2"); // optional
//   {
//       Prof prof(TAG1);
//       foo1();
//   }
//   {
//       Prof prof(TAG1);
//       foo2();
//   }
//   {
//       Prof prof(TAG2);
//       bar();
//   }
//   mgr.Report();

class Prof;

class ProfMgr {
public:
    enum Mode
    {
        SINGLE_THREAD,
        MULTI_THREAD,
    };

    // don't set thrdIdCb, it is for future use
    ProfMgr(Mode mode, int tag_count = 1, int (*thrdIdCb)(void) = NULL);
    void Report() const;
    int get_tag_count()          const { return tag_count_; }
    Timespec get_mgr_init_time() const { return mgr_init_time_; }
    void add_tag_name(int tag, std::string name) { assert(tag < tag_count_); tag_names_[tag] = name; }

    // for Prof to call, not for ProfMgr user
    void AddProf(Prof const *prof);

private:
    void PrintTagTotalNs(int tag, int64_t period_ns) const;

    Mode mode_;
    int tag_count_;
    int (*thread_id_callback_)(void);
    Timespec mgr_init_time_;

    static const int kPrime = 9887;
    std::vector<std::vector<int64_t> > tag_periods_;
    std::vector<std::string> tag_names_;
};

// Prof will calculate the time period from ctor to dtor.
// If sProfMgr_ is set, it will add a record to ProfMgr in dtor.
// Example 1:
// Prof can be used standalone without ProfMgr.
// If a string title is set, it will output profiling msg instantly when dtor is called.
//   {
//       Prof prof("title") // ctor called
//       a_function_to_be_profiled();
//   }  // prof's dtor is called
//
// Example 2:
// Using a int tag, the same tag means it is will be merged.
//   {
//       Prof prof(1)
//       func1_to_be_profiled();
//   }
//   {
//       Prof prof(1)
//       func2_to_be_profiled();
//   }
class Prof {
public:
    Prof(int tag);
    Prof(char const *title);
    ~Prof();

    char const *get_title() const { return title_; }
    int get_tag()           const { return tag_; }
    Timespec get_start()    const { return start_; }
    Timespec get_end()      const { return end_; }
    int64_t get_start_ns()  const { return get_ns(start_); }
    int64_t get_end_ns()    const { return get_ns(end_); }
    int64_t get_period_ns() const { return get_end_ns() - get_start_ns(); }

    // for ProfMgr to call, not for Prof user
    static void set_prof_mgr(ProfMgr *mgr)  { assert(!sProfMgr_); sProfMgr_ = mgr; }

private:
    char const *title_;
    int tag_;
    Timespec start_;
    Timespec end_;

    static ProfMgr *sProfMgr_;
};

} // namespace art_profiler

#endif  // ARTONINC_PROFILE_H_
