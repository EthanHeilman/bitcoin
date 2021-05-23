#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

#include <boost/functional/hash.hpp>

#include <algorithm>
#include <random>

#include <chrono>
#include <thread>

#include <iostream>
#include <math.h> 

#include <util/time.h>
#include <crypto/sha512.h>

#include <iomanip> // For std::setprecision s

#ifdef WIN32
#include <sys/types.h> // must go before a number of other headers
#include <windows.h>
#endif


#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

using namespace std;

BOOST_FIXTURE_TEST_SUITE(rng_tests, BasicTestingSetup)


bool cmp(pair<int64_t, unsigned long long>& a, pair<int64_t, unsigned long long>& b)
{
    return a.second > b.second;
}

double Cnt(vector<pair<int64_t, unsigned long long>>* lst)
{
    unsigned long long cnt = 0;
    for (auto& it : *lst) {
        cnt += it.second;
    }
    return cnt;
}

double Sum(vector<pair<int64_t, unsigned long long>>* lst)
{
    unsigned long long sum = 0;
    for (auto& it : *lst) {
        sum += it.first*it.second;
    }
    return sum;
}

double Avg(vector<pair<int64_t, unsigned long long>>* lst)
{
    unsigned long long cnt = Cnt(lst);
    unsigned long long sum = Sum(lst);
    return double(sum)/double(cnt);
}

double Var(vector<pair<int64_t, unsigned long long>>* lst)
{
    double div = 0.0;
    double avg = Avg(lst);
    unsigned long long cnt = 0;

    for (auto& it : *lst) {
        div += pow(it.first-avg, 2)*it.second;
    }
    return div/(cnt-1); // Use Bessel's correction since we unlikely to have the whole pop
}

double StdDiv(vector<pair<int64_t, unsigned long long>>* lst)
{
    double var = Var(lst);
    return sqrt(var);
}

struct Stats
{
  double guessEnt;
  double stdDiv;
  double shanEnt;
  double avg;
  int64_t min;
  int64_t max;
  int64_t span;
  int uniqs;
  unsigned long long cnt;
};


void PrintStats(Stats s)
{
    cout << "GE: " << s.guessEnt << ", stdDiv: " << s.stdDiv <<  ", shanEnt: " << s.shanEnt << ", avg: " << s.avg 
    << ", min:" << s.min << ", max " << s.max << ", span: " << s.span << ", uniqs: " << s.uniqs << endl;
}

void OneSecLatexHeader()
{
    cout << "\\begin{table}[ht] % Latex" << endl;
    cout << "\\small % Latex" << endl;
    cout << "\\begin{center} % Latex" << endl;
    cout << "\\begin{tabular}{|l|r|r|r|r|r|r|} % Latex" << endl;
    cout<< "\\hline % Latex" << endl;
    cout << std::setprecision(2);

    cout << "clock type" << " & " << "per ms (avg)" << " & " << "avg" << " & " << "sleep time" << " & "
    << "span" << " & " <<  "uniqs \\\\ \\hline % Latex" << endl;
}

void OneSecLatexRow(string name, Stats s, int sleepTime)
{
    cout << name << " & " << double(s.avg)/double(sleepTime) << " & " << s.avg << " & " << sleepTime << " & " << s.span
     << " & " << s.uniqs << " \\\\ \\hline % Latex" << endl;
}


void LatexHeader()
{
    cout << "\\begin{table}[ht] % Latex" << endl;
    cout << "\\small % Latex" << endl;
    cout << "\\begin{center} % Latex" << endl;
    cout << "\\begin{tabular}{|l|r|r|r|r|r|r|r|r|} % Latex" << endl;
    cout<< "\\hline % Latex" << endl;
    cout << std::setprecision(2);

    cout << "clock type" << " & " << "g ent" << " & " << "stdDiv" << " & " << "s ent" << " & "
    << "avg." << " & " <<  "min" << " & " << "max" << " & " << "span" << " & " <<  "uniqs" << " \\\\ \\hline % Latex" << endl;
}
void LatexRow(string name, Stats s)
{
    cout << name << " & " << s.guessEnt << " & " << s.stdDiv <<  " & " << s.shanEnt << " & " << s.avg 
    << " & " << s.min << " & " << s.max << " & " << s.span << " & " << s.uniqs << " \\\\ \\hline % Latex" << endl;
}

void LatexFooter(string label, string caption)
{
    cout << "\\end{tabular} % Latex" << endl;
    cout << "\\label{tbl:"<<label<<"} % Latex" << endl;
    cout << "\\caption{"<<caption<<"} % Latex" << endl;
    cout << "\\end{center} % Latex" << endl;
    cout << "\\end{table} % Latex" << endl << endl;
}

double ComputeShannonEntropy(double stdDiv)
{
    // 1/2 log2(2 pi stdDiv^2) + 1/2
    return 0.5*log2(2.0 * M_PI * pow(stdDiv, 2)) + 0.5;
}

Stats GuessEnt(unordered_map<int64_t, unsigned long long> map)
{
    unordered_map<int64_t, unsigned long long>::iterator it;

    vector<pair<int64_t, unsigned long long> > lst;

    for (auto& it : map) {
        lst.push_back(it);
    }

    sort(lst.begin(), lst.end(), cmp);

    unsigned long long cnt = Cnt(&lst);
    double avg = Avg(&lst);
    double stdDiv = StdDiv(&lst);

    int64_t min = lst[0].first;
    int64_t max = lst[0].first;

    double gent = 0.0;
    int64_t i = 0;
    for (auto& it : lst)
    {
        if (it.first < min) min = it.first;
        if (it.first > max) max = it.first;

        gent += i*(double(it.second)/double(cnt));
        i++;

    }

    Stats s;
    s.guessEnt = gent;
    s.stdDiv = stdDiv;
    s.shanEnt = ComputeShannonEntropy(stdDiv);
    s.avg = avg;
    s.min = min;
    s.max = max;
    s.span = max - min;
    s.uniqs = map.size();
    s.cnt = cnt;

    return s;
}

template <typename Container> // we can make this generic for any container [1]
struct container_hash {
    std::size_t operator()(Container const& c) const {
        return boost::hash_range(c.begin(), c.end());
    }
};

bool vcmp(pair<vector<int64_t>, unsigned long long>& a, pair<vector<int64_t>, unsigned long long>& b)
{
    return a.second > b.second;
}

void PrintVMap(unordered_map<vector<int64_t>, unsigned long long, container_hash<vector<int64_t>>> map, string name)
{
    vector<pair<vector<int64_t>, unsigned long long> > lst;

    for (auto& it : map) {
        lst.push_back(it);
    }

    sort(lst.begin(), lst.end(), vcmp);

    cout << name << " = [";
    for (unsigned long i = 0; i < lst.size(); i++)
    {
        vector <int64_t> key_vector = lst[i].first;
        for(size_t j=0; j < key_vector.size(); j++)
        {
            cout << key_vector[j];
            if (j < key_vector.size()-1)
                cout << "-";
        }

        cout << " : " << lst[i].second << endl;

    }

}


void PrintMap(unordered_map<int64_t, unsigned long long> map, string name)
{
    vector<pair<int64_t, unsigned long long> > lst;

    for (auto& it : map) {
        lst.push_back(it);
    }

    sort(lst.begin(), lst.end(), cmp);

    int rowLen = 12;
    int pos = 0;

    cout << name << " = [";
    // for (auto& it : lst)
    for (unsigned long i = 0; i < lst.size(); i++)
    {
        cout << lst[i].first << ":" << lst[i].second;

        if (i != (lst.size()-1)) cout<<", ";
        if (pos > 0  && pos % rowLen == 0) cout << endl;

        pos++;
    }
    cout << "]" << endl << endl;
}

BOOST_AUTO_TEST_CASE(time_perfcounter_benchmark_test)
{
    unsigned long long n1 = 60;
    unsigned long long n2 = 1;

    Stats s;

    cout << "1 Second GetPerformanceCounter Clocks" << endl;
    cout << "====" << endl;

    OneSecLatexHeader();

    for(int ms = 500; ms <= 2000; ms+=500)
        {
        unordered_map<int64_t, unsigned long long> pcmap;
        string clockUsed = "None";

    #if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
        clockUsed = "rdtscMSC";
        int64_t t1 = 0;
        int64_t t2 = 0;
        for (unsigned long long i=0; i< n1; i++)
        {
            for (unsigned long long j=0; j< n2; j++)
            {     
                t1 = __rdtsc();
                std::this_thread::sleep_for(std::chrono::milliseconds(ms)); // sleep for 1 second
                t2 = __rdtsc();
                pcmap[t2-t1] += 1;
            }
        }
    #elif !defined(_MSC_VER) && defined(__i386__)
        clockUsed = "rdtsci386";

        uint64_t t1 = 0;
        uint64_t t2 = 0;
        for (unsigned long long i=0; i< n1; i++)
        {
            for (unsigned long long j=0; j< n2; j++)
            {     
                __asm__ volatile ("rdtsc" : "=A"(t1));
                std::this_thread::sleep_for(std::chrono::milliseconds(ms)); // sleep for 1 second
                __asm__ volatile ("rdtsc" : "=A"(t2));
                pcmap[t2-t1] += 1;
            }
        }
    #elif !defined(_MSC_VER) && (defined(__x86_64__) || defined(__amd64__))
        clockUsed = "rdtsc64";

        int64_t t1 = 0;
        uint64_t t1a = 0,t1b = 0;
        int64_t t2 = 0;
        uint64_t t2a = 0,t2b = 0;
        for (unsigned long long i=0; i< n1; i++)
        {
            for (unsigned long long j=0; j< n2; j++)
            {  
                __asm__ volatile ("rdtsc" : "=a"(t1a), "=d"(t1b)); // Constrain r1 to rax and r2 to rdx.
                std::this_thread::sleep_for(std::chrono::milliseconds(ms)); // sleep for 1 second
                __asm__ volatile ("rdtsc" : "=a"(t2a), "=d"(t2b)); // Constrain r1 to rax and r2 to rdx.

                t1 =  (t1b << 32) | t1a;
                t2 =  (t2b << 32) | t2a;
                pcmap[t2-t1] += 1;
            }
        }
    #else
        clockUsed = "std::chrono::HRC";

        int64_t t1 = 0;
        int64_t t2 = 0;
        for (unsigned long long i=0; i< n1; i++)
        {
            for (unsigned long long j=0; j< n2; j++)
            {  
                t1  = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                std::this_thread::sleep_for(std::chrono::milliseconds(ms)); // sleep for 1 second
                t2 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                pcmap[t2-t1] += 1;
            }
        }
    #endif
        // s = GuessEnt(pcmap);
        OneSecLatexRow(clockUsed + "for" + std::to_string(ms)+"ms", s, ms);
        PrintMap(pcmap, clockUsed + "for" + std::to_string(ms)+"ms");
    }
    LatexFooter("1secperfclockbenchmark", "This measures how much the perf counter moves in 1 second.");
}

BOOST_AUTO_TEST_CASE(perfcounter_benchmark_test)
{
    unsigned long long n1 = 1000*1000;
    unsigned long long n2 = 1000;
    // unsigned long long n1 = 5;
    // unsigned long long n2 = 1;
    Stats s;

    cout << "GetPerformanceCounter Clocks" << endl;
    cout << "====" << endl;

    LatexHeader();

    unordered_map<int64_t, unsigned long long> pcmap;
    string clockUsed = "None";

#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
    clockUsed = "rdtsc MSC";
    int64_t t1 = 0;
    int64_t t2 = 0;
    for (unsigned long long i=0; i< n1; i++)
    {
        for (unsigned long long j=0; j< n2; j++)
        {     
            t1 = __rdtsc();
            t2 = __rdtsc();
            pcmap[t2-t1] += 1;
        }
    }
#elif !defined(_MSC_VER) && defined(__i386__)
    clockUsed = "rdtsc i386";

    uint64_t t1 = 0;
    uint64_t t2 = 0;
    for (unsigned long long i=0; i< n1; i++)
    {
        for (unsigned long long j=0; j< n2; j++)
        {     
            __asm__ volatile ("rdtsc" : "=A"(t1));
            __asm__ volatile ("rdtsc" : "=A"(t2));
            pcmap[t2-t1] += 1;
        }
    }
#elif !defined(_MSC_VER) && (defined(__x86_64__) || defined(__amd64__))
    clockUsed = "rdtsc64";

    int64_t t1 = 0;
    uint64_t t1a = 0,t1b = 0;
    int64_t t2 = 0;
    uint64_t t2a = 0,t2b = 0;
    for (unsigned long long i=0; i< n1; i++)
    {
        for (unsigned long long j=0; j< n2; j++)
        {  
            __asm__ volatile ("rdtsc" : "=a"(t1a), "=d"(t1b)); // Constrain r1 to rax and r2 to rdx.
            __asm__ volatile ("rdtsc" : "=a"(t2a), "=d"(t2b)); // Constrain r1 to rax and r2 to rdx.

            t1 =  (t1b << 32) | t1a;
            t2 =  (t2b << 32) | t2a;
            pcmap[t2-t1] += 1;
        }
    }
#else
    clockUsed = "HRC";

    int64_t t1 = 0;
    int64_t t2 = 0;
    for (unsigned long long i=0; i< n1; i++)
    {
        for (unsigned long long j=0; j< n2; j++)
        {  
            t1  = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            t2 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            pcmap[t2-t1] += 1;
        }
    }
#endif
    // s = GuessEnt(pcmap);
    // cout << clockUsed << ", ";
    // PrintStats(s);
    PrintMap(pcmap, "perfcounter"+clockUsed);
    LatexRow(clockUsed, s);
    LatexFooter("perfclockbenchmark", "This is a benchmark of the performance counter");
}

BOOST_AUTO_TEST_CASE(clock_benchmark_test)
{
    cout << "Dyn Env Clocks" << endl;
    cout << "====" << endl;

    unsigned long long n1 = 1000*1000;
    unsigned long long n2 = 1000;
    // unsigned long long n1 = 5;
    // unsigned long long n2 = 1;

    Stats s;

    LatexHeader();
#ifdef WIN32
    unordered_map<int64_t, unsigned long long> ftimemap;
    FILETIME ftime1;
    FILETIME ftime2;

    for (unsigned long long i=0; i< n1; i++)
    {
        for (unsigned long long j=0; j< n2; j++)
        {     
            GetSystemTimeAsFileTime(&ftime1);
            GetSystemTimeAsFileTime(&ftime2);
            unsigned long long ullftime1 = (((unsigned long long) ftime1.dwHighDateTime) << 32) + ftime1.dwLowDateTime;
            unsigned long long ullftime2 = (((unsigned long long) ftime2.dwHighDateTime) << 32) + ftime2.dwLowDateTime;
            int64_t ftimediff = ullftime2 - ullftime1;
            // ftimemap[ftime2-ftime1] += 1;
            ftimemap[ftimediff] += 1;
        }
    }
    s = GuessEnt(ftimemap);
    // cout << "GetSystemTimeAsFileTime: ";
    // PrintStats(s);
    PrintMap(ftimemap, "GetSystemTimeAsFileTime");
    LatexRow("GetSystemTimeAsFileTime", s);


#else

    struct timespec ts1 = {};
    struct timespec ts2 = {};

#    ifdef CLOCK_MONOTONIC
    unordered_map<int64_t, unsigned long long> tsmonomap;
    for (unsigned long long i=0; i< n1; i++)
    {
        for (unsigned long long j=0; j< n2; j++)
        {     
            clock_gettime(CLOCK_MONOTONIC, &ts1);
            clock_gettime(CLOCK_MONOTONIC, &ts2);

            unsigned long long diff = (ts2.tv_sec-ts1.tv_sec)*1000*1000*1000 + (ts2.tv_nsec-ts1.tv_nsec);
            tsmonomap[diff] += 1;
        }
    }
    s = GuessEnt(tsmonomap);
    PrintMap(tsmonomap, "CLOCK\\_MONOTONIC");
    LatexRow("CLOCK\\_MONOTONIC", s);


#    endif
#    ifdef CLOCK_REALTIME
    unordered_map<int64_t, unsigned long long> tsrealmap;
    for (unsigned long long i=0; i< n1; i++)
    {
        for (unsigned long long j=0; j< n2; j++)
        { 
            clock_gettime(CLOCK_REALTIME, &ts1);
            clock_gettime(CLOCK_REALTIME, &ts2);

            unsigned long long diff = (ts2.tv_sec-ts1.tv_sec)*1000*1000*1000 + (ts2.tv_nsec-ts1.tv_nsec);
            tsrealmap[diff] += 1;
        }
    }
    s = GuessEnt(tsrealmap);
    PrintMap(tsrealmap, "CLOCK\\_REALTIME");
    LatexRow("CLOCK\\_REALTIME", s);

#    endif
#    ifdef CLOCK_BOOTTIME
    unordered_map<int64_t, unsigned long long> tsbootmap;
    for (unsigned long long i=0; i< n1; i++)
    {
        for (unsigned long long j=0; j< n2; j++)
        { 
            clock_gettime(CLOCK_BOOTTIME, &ts1);
            clock_gettime(CLOCK_BOOTTIME, &ts2);

            unsigned long long diff = (ts2.tv_sec-ts1.tv_sec)*1000*1000*1000 + (ts2.tv_nsec-ts1.tv_nsec);
            tsbootmap[diff] += 1;
        }
    }
    s = GuessEnt(tsbootmap);
    PrintMap(tsbootmap, "CLOCK\\_BOOTTIME");
    LatexRow("CLOCK\\_BOOTTIME", s);

#    endif
    // gettimeofday is available on all UNIX systems, but only has microsecond precision.
    struct timeval tv1 = {};
    struct timeval tv2 = {};
    unordered_map<int64_t, unsigned long long> gettodmap;
    for (unsigned long long i=0; i< n1; i++)
    {
        for (unsigned long long j=0; j< n2; j++)
        { 
            gettimeofday(&tv1, nullptr);
            gettimeofday(&tv2, nullptr);

            // tv_sec
            unsigned long long diff = (tv2.tv_sec-tv1.tv_sec)*1000*1000 + (tv2.tv_usec-tv1.tv_usec);
            gettodmap[diff] += 1;
        }
    }
    s = GuessEnt(gettodmap);
    PrintMap(gettodmap, "gettimeofday");
    LatexRow("gettimeofday", s);



#endif

    unsigned long long sysepoch1;
    unsigned long long sysepoch2;
    unordered_map<int64_t, unsigned long long> sysepochmap;

    for (unsigned long long i=0; i< n1; i++)
    {
        for (unsigned long long j=0; j< n2; j++)
        { 
            sysepoch1 = std::chrono::system_clock::now().time_since_epoch().count();
            sysepoch2 = std::chrono::system_clock::now().time_since_epoch().count();

            sysepochmap[sysepoch2-sysepoch1] += 1;
        }
    }
    s = GuessEnt(sysepochmap);
    PrintMap(sysepochmap, "system_clock");
    LatexRow("std::chrono::system\\_clock", s);


    unsigned long long steadyepoch1;
    unsigned long long steadyepoch2;
    unordered_map<int64_t, unsigned long long> steadymap;

    for (unsigned long long i=0; i< n1; i++)
    {
        for (unsigned long long j=0; j< n2; j++)
        {
            steadyepoch1 = std::chrono::steady_clock::now().time_since_epoch().count();
            steadyepoch2 = std::chrono::steady_clock::now().time_since_epoch().count();

            steadymap[steadyepoch2-steadyepoch1] += 1;
        }
    }
    s = GuessEnt(steadymap);
    PrintMap(steadymap, "steady_clock");
    LatexRow("std::chrono::steady\\_clock", s);


    unsigned long long hrcepoch1;
    unsigned long long hrcepoch2;
    unordered_map<int64_t, unsigned long long> hrcepochmap;

    for (unsigned long long i=0; i< n1; i++)
    {
        for (unsigned long long j=0; j< n2; j++)
        { 
            hrcepoch1 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            hrcepoch2 = std::chrono::high_resolution_clock::now().time_since_epoch().count();

            hrcepochmap[hrcepoch2-hrcepoch1] += 1;
        }
    }
    s = GuessEnt(hrcepochmap);
    PrintMap(hrcepochmap, "HRC");
    LatexRow("HRC", s);
    LatexFooter("dynenvclocks", "Clocks used in dyn evn");


}

enum PERFINSTR { sysdefault, hrc };

static inline int64_t GetPerformanceCounterHRC() noexcept
{
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

static inline int64_t GetPerformanceCounterSysdefault() noexcept
{
    // Read the hardware time stamp counter when available.
    // See https://en.wikipedia.org/wiki/Time_Stamp_Counter for more information.
    #if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
        return __rdtsc();
    #elif !defined(_MSC_VER) && defined(__i386__)
        uint64_t r = 0;
        __asm__ volatile ("rdtsc" : "=A"(r)); // Constrain the r variable to the eax:edx pair.
        return r;
    #elif !defined(_MSC_VER) && (defined(__x86_64__) || defined(__amd64__))
        uint64_t r1 = 0, r2 = 0;
        __asm__ volatile ("rdtsc" : "=a"(r1), "=d"(r2)); // Constrain r1 to rax and r2 to rdx.
        return (r2 << 32) | r1;
    #else
        // Fall back to using C++11 clock (usually microsecond or nanosecond precision)
        return std::chrono::high_resolution_clock::now().time_since_epoch().count();
    #endif
}

// Instrumented version of strengthen to capture:
// perf per cycle (loop)
// number of cycles
static void StrengthenInst(const unsigned char (&seed)[32], int microseconds, CSHA512& hasher,
vector<int64_t>& vperf, int& cycle
) noexcept
{
    cycle = 0;
    CSHA512 inner_hasher;
    inner_hasher.Write(seed, sizeof(seed));

    // Hash loop
    unsigned char buffer[64];
    int64_t stop = GetTimeMicros() + microseconds;
    vperf.push_back(GetPerformanceCounterSysdefault());
    do {
        for (int i = 0; i < 1000; ++i) {
            inner_hasher.Finalize(buffer);
            inner_hasher.Reset();
            inner_hasher.Write(buffer, sizeof(buffer));
        }
        // Benchmark operation and feed it into outer hasher.
        int64_t perf = GetPerformanceCounterSysdefault();
        //ERH -- Added code
        vperf.push_back(perf);
        cycle++;
        // -- 
        hasher.Write((const unsigned char*)&perf, sizeof(perf));
    } while (GetTimeMicros() < stop);

    // Produce output from inner state and feed it to outer hasher.
    inner_hasher.Finalize(buffer);
    hasher.Write(buffer, sizeof(buffer));
    // Try to clean up.
    inner_hasher.Reset();
    memory_cleanse(buffer, sizeof(buffer));
}


static void StrengthenInstHRC(const unsigned char (&seed)[32], int microseconds, CSHA512& hasher,
vector<int64_t>& vperf, int& cycle
) noexcept
{
    cycle = 0;
    CSHA512 inner_hasher;
    inner_hasher.Write(seed, sizeof(seed));

    // Hash loop
    unsigned char buffer[64];
    int64_t stop = GetTimeMicros() + microseconds;
    vperf.push_back(GetPerformanceCounterHRC());
    do {
        for (int i = 0; i < 1000; ++i) {
            inner_hasher.Finalize(buffer);
            inner_hasher.Reset();
            inner_hasher.Write(buffer, sizeof(buffer));
        }
        // Benchmark operation and feed it into outer hasher.
        int64_t perf = GetPerformanceCounterHRC();
        //ERH -- Added code
        vperf.push_back(perf);
        cycle++;
        // -- 
        hasher.Write((const unsigned char*)&perf, sizeof(perf));
    } while (GetTimeMicros() < stop);

    // Produce output from inner state and feed it to outer hasher.
    inner_hasher.Finalize(buffer);
    hasher.Write(buffer, sizeof(buffer));
    // Try to clean up.
    inner_hasher.Reset();
    memory_cleanse(buffer, sizeof(buffer));
}


static void RunStrengthenInst(int microseconds, vector<int64_t>& perfarray, int& cycle, PERFINSTR instr)
{
    unsigned char seed[32];
    GetRandBytes(seed, 32);
    CSHA512 hasher;

    switch (instr)
    {
        case PERFINSTR::sysdefault:
        {
            StrengthenInst(seed, microseconds, hasher, perfarray, cycle);
            break;
        }
        case PERFINSTR::hrc:
        {
            StrengthenInstHRC(seed, microseconds, hasher, perfarray, cycle);
            break;
        }
        default:
        {
            assert(0);
        }
    };
}


string GetSystemParams()
{
    #if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
        return "rdtscMSC";
    #elif !defined(_MSC_VER) && defined(__i386__)
        return  "rdtsci386";
    #elif !defined(_MSC_VER) && (defined(__x86_64__) || defined(__amd64__))
        return "rdtsc64";
    #else
        return "None";
    #endif
}



void StrengthenLatexHeader()
{
    cout << "\\begin{table}[ht] % Latex" << endl;
    cout << "\\small % Latex" << endl;
    cout << "\\begin{center} % Latex" << endl;
    cout << "\\begin{tabular}{|l|r|r|r|r|r|r|r|r|} % Latex" << endl;
    cout<< "\\hline % Latex" << endl;
    cout << std::setprecision(2);

    cout << "clock type" << " & " << "g ent" << " & " << "cycles (min,max,avg)" << " & " << "stdDiv" << " & " 
    << "avg." << " & " <<  "min" << " & " << "max" << " & " << "span" << " & " <<  "uniqs" << " \\\\ \\hline % Latex" << endl;
}
void StrengthenLatexRow(string name, Stats s, int cyclesMin, int cyclesMax, int cyclesAvg)
{
    cout << name << " & " << s.guessEnt << " & " <<  std::to_string(cyclesMin) + "-" + std::to_string(cyclesMax) + "-" + std::to_string(cyclesAvg) << " & " << s.stdDiv  << " & " << s.avg 
    << " & " << s.min << " & " << s.max << " & " << s.span << " & " << s.uniqs << " \\\\ \\hline % Latex" << endl;
}

void StrengthenLatexFooter(string label, string caption)
{
    cout << "\\end{tabular} % Latex" << endl;
    cout << "\\label{tbl:"<<label<<"} % Latex" << endl;
    cout << "\\caption{"<<caption<<"} % Latex" << endl;
    cout << "\\end{center} % Latex" << endl;
    cout << "\\end{table} % Latex" << endl << endl;
}


static void GuessEntropyStrengthen(int microseconds, int nSamples, PERFINSTR instr)
{
    int count = 0;
    int minCycles = -1;
    int maxCycles = 0;
    double avgCycles = 0;

    //TODO: add average cycles

    std::unordered_map<int64_t, unsigned long long> perfsMap;
    vector<int> cycles;

    std::unordered_map<int64_t, unsigned long long> cycleMap;

    std::unordered_map<vector<int64_t>,  unsigned long long, container_hash<vector<int64_t>>> betterMap;


    for (int i = 0; i < nSamples; i++)
    {
        // contains all the performance counter values
        vector<int64_t> perfarray;
        vector<int64_t> betterarray;

        int cycle = 0;

        RunStrengthenInst(microseconds, perfarray, cycle, instr);
        cycles.push_back(cycle);
        cycleMap[cycle] += 1;
        for (int64_t j = 1; j < cycle+1; j++)
        {
            int64_t timediff = perfarray[j]-perfarray[j-1];
            perfsMap[timediff] += 1;
            betterarray.push_back(timediff);
            count+=1;
        }

        if (minCycles == -1 || minCycles > cycle) minCycles = cycle;
        if (maxCycles < cycle) maxCycles = cycle;

        avgCycles += cycle;

        betterMap[betterarray] += 1;
    }
    avgCycles = avgCycles/nSamples;

    // Stats s = GuessEnt(perfsMap);
    string name;
    switch (instr)
    {
        case PERFINSTR::sysdefault:
        {
            name = GetSystemParams()+"for"+std::to_string(microseconds/1000)+"ms";
            break;
        }
        case PERFINSTR::hrc:
        {
            name = "HRClockfor"+std::to_string(microseconds/1000)+"ms";
            break;
        }
        default:
        {
            assert(0);
        }
    };

    PrintMap(perfsMap, name);
    PrintVMap(betterMap, name);
    PrintMap(cycleMap, name+"_cyclemap");

    size_t rowLen = 10;
    cout << "cycles = [";
    for( size_t i=0; i<cycles.size(); ++i)
    {
        std::cout << cycles[i];
        if (i != (cycles.size()-1)) cout<<", ";
        if (i > 0  && i % rowLen == 0) cout << endl;
    }
    cout << "]" << endl << endl;


    // StrengthenLatexRow(name, s, minCycles, maxCycles, avgCycles);
}


BOOST_AUTO_TEST_CASE(strengthen_benchmark_test)
{
    cout << "Strengthen Benchmark" << endl;
    cout << "====" << endl;

    int microseconds = 10*1000;
    int seconds_to_run = 60*60;
    int nSample = 100*seconds_to_run;
    
    // StrengthenLatexHeader();

    GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::sysdefault);
    GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::hrc);

    microseconds = 100*1000;
    nSample = 10*seconds_to_run;

    GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::sysdefault);
    GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::hrc);

    StrengthenLatexFooter("strengthen", "Measurements of strengthen");

}

BOOST_AUTO_TEST_SUITE_END()
