#include <time.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <cassert>
#include <string>

using namespace std;

// TODO: use namespace

typedef struct timespec Timespec;
class ProfMgr;
class Prof;

void print_time(long long ns, char const *title)
{
    int sec = ns / 1000000000LL;

    int ss = sec % 60;
    int mm = sec / 60 % 60;
    int hh = sec / 60 / 60;
    int ms = ns / 1000000 % 1000;

    printf("%s - %02d:%02d:%02d.%03d \n", title, hh, mm, ss, ms);
}

long long getNs(Timespec const &s)
{
    return s.tv_sec * 1000000000LL + s.tv_nsec;
}

// ===================================
// Declaration
// ===================================

class ProfMgr {
private:
    struct InnerProf {
        char const *_title;
        Timespec _start;
        Timespec _end;
    };

public:
    enum Mode
    {
        SINGLE_THREAD,
        MULTI_THREAD,
    };

    ProfMgr(Mode mode, int (*thrdIdCb)(void) = NULL);
    void addProf(char const *title, Timespec s, Timespec e);
    void report(bool mergeTitle);

private:
    string ns2str(long long ns);
    void printOneRecord(char const *title, Timespec const &s, Timespec const &e);

    Mode _mode;
    int (*_thrdIdCb)(void);
    Timespec _mgrInit;

    vector<vector<InnerProf> > _profs;
};

class Prof {
public:
    Prof(char const *title);
    ~Prof();

    char const *getTitle();
    Timespec getStart();
    Timespec getEnd();
    long long getStartNs();
    long long getEndNs();
    long long getPeriodNs();

    // cannot reset
    static void setProfMgr(ProfMgr *mgr);

private:
    char const *_title;
    Timespec _start;
    Timespec _end;

    static ProfMgr *_sProfMgr;
};

// ===================================
// Definition
// ===================================

ProfMgr *Prof::_sProfMgr = NULL;

Prof::Prof(char const *title)
{
    assert(_sProfMgr);
    _title = title;
    clock_gettime(CLOCK_REALTIME, &_start);
}

Prof::~Prof()
{
    clock_gettime(CLOCK_REALTIME, &_end);
    assert(_sProfMgr);
    _sProfMgr->addProf(_title, _start, _end);
}

char const *Prof::getTitle()     { return _title; }
Timespec Prof::getStart()        { return _start; }
Timespec Prof::getEnd()          { return _end; }
long long Prof::getStartNs()     { return getNs(_start); }
long long Prof::getEndNs()       { return getNs(_end); }
long long Prof::getPeriodNs()    { return getEndNs() - getStartNs(); }
/* static */ void Prof::setProfMgr(ProfMgr *mgr) { assert(!_sProfMgr); _sProfMgr = mgr; }

static const int SLOT = 10000;
ProfMgr::ProfMgr(Mode mode, int (*thrdIdCb)(void)) 
    : _mode(mode)
    , _thrdIdCb(thrdIdCb)
    , _profs(SLOT, vector<InnerProf>())
{
    clock_gettime(CLOCK_REALTIME, &_mgrInit);
    Prof::setProfMgr(this);
    for (int i = 0; i < SLOT; ++i) {
        _profs[i].reserve(100);
    }
}

void ProfMgr::addProf(char const *title, Timespec s, Timespec e)
{
    int i = -1;
    if (_mode == SINGLE_THREAD) {
        i = 0;
    } else if (_thrdIdCb) {
        i = _thrdIdCb();
    } else {
        Timespec tmp;
        clock_gettime(CLOCK_REALTIME, &tmp);
        i = tmp.tv_nsec % 9887; // pseudo random
    }
    assert(-1 < i && i < SLOT);
    InnerProf p = {title, s, e};
    if (0) { // debug
        printf("(debug) %s %lld %lld \n", title, getNs(s), getNs(e));
    }
    _profs[i].push_back(p);
}

string ProfMgr::ns2str(long long ns)
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

void ProfMgr::printOneRecord(char const *title, Timespec const &s, Timespec const &e)
{
    long long beginNs = getNs(s) - getNs(_mgrInit);
    long long periodNs = getNs(e) - getNs(s);
    printf("%25s ] start at: %s, period: %s \n",
            title,
            ns2str(beginNs).c_str(),
            ns2str(periodNs).c_str());
}

void ProfMgr::report(bool mergeTitle)
{
    if (_mode == SINGLE_THREAD) {
        for (int i = 0; i < _profs[0].size(); ++i) {
            InnerProf &iprof = _profs[0][i];
            printOneRecord(iprof._title, iprof._start, iprof._end);
        }
    }
}

// ===================================
// Main
// ===================================

void foo() {
    long long sum = 0;
    int j = 0;
    for (int i = 0; i < 100000000; ++i) {
        if (++j % 37 == 0) continue;
        sum += i;
    }
    printf("sum = %lld \n", sum);
}

int main() {
    ProfMgr mgr(ProfMgr::SINGLE_THREAD);
    {
        Prof prof("foo");
        foo();
    }
    {
        Prof prof("foo");
        foo();
    }
    mgr.report(false);

    return 0;
}

