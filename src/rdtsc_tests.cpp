#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <random>

#include <chrono>
#include <thread>

#include <iostream>
using namespace std;

BOOST_FIXTURE_TEST_SUITE(rdtsc_tests, BasicTestingSetup)

void measure_rdtsc_base(uint64_t* s)
{
    uint64_t r1a = 0, r2a = 0;
    uint64_t r1b = 0, r2b = 0;
    uint64_t r1c = 0, r2c = 0;
    uint64_t r1d = 0, r2d = 0;
    uint64_t r1e = 0, r2e = 0;

    __asm__ volatile ("rdtsc" : "=a"(r1a), "=d"(r2a));
    __asm__ volatile ("rdtsc" : "=a"(r1b), "=d"(r2b));

    // for (int i = 0; i< 1; i++)
    // {
    //     s[0] = 0x6a09e667f3bcc908ull + i + s[1] + s[7];
    //     s[1] = 0xbb67ae8584caa73bull + i + s[7];
    //     s[2] = 0x3c6ef372fe94f82bull + i + s[6];
    //     s[3] = 0xa54ff53a5f1d36f1ull + i + s[5];
    //     s[4] = 0x510e527fade682d1ull + i + s[4];
    //     s[5] = 0x9b05688c2b3e6c1full + i + s[3];
    //     s[6] = 0x1f83d9abfb41bd6bull + i + s[2];
    //     s[7] = 0x5be0cd19137e2179ull + i + s[1];
    // }
    __asm__ volatile ("rdtsc" : "=a"(r1c), "=d"(r2c));
    __asm__ volatile ("rdtsc" : "=a"(r1d), "=d"(r2d));
    __asm__ volatile ("rdtsc" : "=a"(r1e), "=d"(r2e));

    std::cout <<"Base case:" << std::endl;
    std::cout <<"========" << std::endl;
    std::cout <<"rdtsc call 1: " << r1a << std::endl;
    std::cout <<"rdtsc call 2: " << r1b << " diff last call: "<< r1b-r1a << std::endl;
    std::cout <<"No Assignment!!!"<<std::endl;
    std::cout <<"rdtsc call 3: " << r1c << " diff last call: "<< r1c-r1b << std::endl;
    std::cout <<"rdtsc call 4: " << r1d << " diff last call: "<< r1d-r1c << std::endl;
    std::cout <<"rdtsc call 5: " << r1e << " diff last call: "<< r1e-r1d << std::endl;
    std::cout <<"     " << std::endl;

}



void measure_rdtsc_reorder(uint64_t* s)
{
    uint64_t r1a = 0, r2a = 0;
    uint64_t r1b = 0, r2b = 0;
    uint64_t r1c = 0, r2c = 0;
    uint64_t r1d = 0, r2d = 0;
    uint64_t r1e = 0, r2e = 0;

    __asm__ volatile ("rdtsc" : "=a"(r1a), "=d"(r2a));
    __asm__ volatile ("rdtsc" : "=a"(r1b), "=d"(r2b));

    for (int i = 0; i< 1; i++)
    {
        s[0] = 0x6a09e667f3bcc908ull + i + s[1] + s[7];
        s[1] = 0xbb67ae8584caa73bull + i + s[7];
        s[2] = 0x3c6ef372fe94f82bull + i + s[6];
        s[3] = 0xa54ff53a5f1d36f1ull + i + s[5];
        s[4] = 0x510e527fade682d1ull + i + s[4];
        s[5] = 0x9b05688c2b3e6c1full + i + s[3];
        s[6] = 0x1f83d9abfb41bd6bull + i + s[2];
        s[7] = 0x5be0cd19137e2179ull + i + s[1];
    }
    __asm__ volatile ("rdtsc" : "=a"(r1c), "=d"(r2c));
    __asm__ volatile ("rdtsc" : "=a"(r1d), "=d"(r2d));
    __asm__ volatile ("rdtsc" : "=a"(r1e), "=d"(r2e));

    std::cout <<"Trigger reorder:" << std::endl;
    std::cout <<"========" << std::endl;
    std::cout <<"rdtsc call 1: " << r1a << std::endl;
    std::cout <<"rdtsc call 2: " << r1b << " diff last call: "<< r1b-r1a << std::endl;
    std::cout <<"Assignment!!!"<<std::endl;
    std::cout <<"rdtsc call 3: " << r1c << " diff last call: "<< r1c-r1b << std::endl;
    std::cout <<"rdtsc call 4: " << r1d << " diff last call: "<< r1d-r1c << std::endl;
    std::cout <<"rdtsc call 5: " << r1e << " diff last call: "<< r1e-r1d << std::endl;
    std::cout <<"     " << std::endl;

}

void measure_rdtsc_no_reorder(uint64_t* s)
{
    uint64_t r1a = 0, r2a = 0;
    uint64_t r1b = 0, r2b = 0;
    uint64_t r1c = 0, r2c = 0;
    uint64_t r1d = 0, r2d = 0;
    uint64_t r1e = 0, r2e = 0;

    __asm__ volatile ("rdtsc" : "=a"(r1a), "=d"(r2a));
    __asm__ volatile ("rdtsc" : "=a"(r1b), "=d"(r2b));

    for (int i = 0; i< 100; i++)
    {
        s[0] = 0x6a09e667f3bcc908ull + i + s[1] + s[7];
        s[1] = 0xbb67ae8584caa73bull + i + s[7];
        s[2] = 0x3c6ef372fe94f82bull + i + s[6];
        s[3] = 0xa54ff53a5f1d36f1ull + i + s[5];
        s[4] = 0x510e527fade682d1ull + i + s[4];
        s[5] = 0x9b05688c2b3e6c1full + i + s[3];
        s[6] = 0x1f83d9abfb41bd6bull + i + s[2];
        s[7] = 0x5be0cd19137e2179ull + i + s[1];
    }
    __asm__ volatile ("rdtsc" : "=a"(r1c), "=d"(r2c));
    __asm__ volatile ("rdtsc" : "=a"(r1d), "=d"(r2d));
    __asm__ volatile ("rdtsc" : "=a"(r1e), "=d"(r2e));

    std::cout <<"Reorder doesn't happen:" << std::endl;
    std::cout <<"========" << std::endl;
    std::cout <<"rdtsc call 1: " << r1a << std::endl;
    std::cout <<"rdtsc call 2: " << r1b << " diff last call: "<< r1b-r1a << std::endl;
    std::cout <<"Assignment 100 times !!!"<<std::endl;
    std::cout <<"rdtsc call 3: " << r1c << " diff last call: "<< r1c-r1b << std::endl;
    std::cout <<"rdtsc call 4: " << r1d << " diff last call: "<< r1d-r1c << std::endl;
    std::cout <<"rdtsc call 5: " << r1e << " diff last call: "<< r1e-r1d << std::endl;
    std::cout <<"     " << std::endl;

}

void measure_rdtsc_mfence(uint64_t* s)
{
    uint64_t r1a = 0, r2a = 0;
    uint64_t r1b = 0, r2b = 0;
    uint64_t r1c = 0, r2c = 0;
    uint64_t r1d = 0, r2d = 0;
    uint64_t r1e = 0, r2e = 0;

    __asm__ volatile ("lfence; rdtsc; lfence" : "=a"(r1a), "=d"(r2a));
    __asm__ volatile ("lfence; rdtsc; lfence" : "=a"(r1b), "=d"(r2b));

    for (int i = 0; i< 100; i++)
    {
        s[0] = 0x6a09e667f3bcc908ull + i + s[1] + s[7];
        s[1] = 0xbb67ae8584caa73bull + i + s[7];
        s[2] = 0x3c6ef372fe94f82bull + i + s[6];
        s[3] = 0xa54ff53a5f1d36f1ull + i + s[5];
        s[4] = 0x510e527fade682d1ull + i + s[4];
        s[5] = 0x9b05688c2b3e6c1full + i + s[3];
        s[6] = 0x1f83d9abfb41bd6bull + i + s[2];
        s[7] = 0x5be0cd19137e2179ull + i + s[1];
    }
    __asm__ volatile ("lfence; rdtsc; lfence" : "=a"(r1c), "=d"(r2c));
    __asm__ volatile ("lfence; rdtsc; lfence" : "=a"(r1d), "=d"(r2d));
    __asm__ volatile ("lfence; rdtsc; lfence" : "=a"(r1e), "=d"(r2e));

    std::cout <<"Prevent reorder with lfence:" << std::endl;
    std::cout <<"========" << std::endl;
    std::cout <<"rdtsc call 1: " << r1a << std::endl;
    std::cout <<"rdtsc call 2: " << r1b << " diff last call: "<< r1b-r1a << std::endl;
    std::cout <<"Assignment!!!"<<std::endl;
    std::cout <<"rdtsc call 3: " << r1c << " diff last call: "<< r1c-r1b << std::endl;
    std::cout <<"rdtsc call 4: " << r1d << " diff last call: "<< r1d-r1c << std::endl;
    std::cout <<"rdtsc call 5: " << r1e << " diff last call: "<< r1e-r1d << std::endl;
    std::cout <<"     " << std::endl;

}

BOOST_AUTO_TEST_CASE(rdtsc_base_test)
{
     uint64_t s[8];
     measure_rdtsc_base(s);
}


BOOST_AUTO_TEST_CASE(rdtsc_reorder_test)
{
     uint64_t s[8];
     measure_rdtsc_reorder(s);
}

BOOST_AUTO_TEST_CASE(rdtsc_noreoder_test)
{
     uint64_t s[8];
     measure_rdtsc_no_reorder(s);
}

BOOST_AUTO_TEST_CASE(rdtsc_mfence_test)
{
     uint64_t s[8];
     measure_rdtsc_mfence(s);
}


static inline int64_t GetPerformanceCounterHRC() noexcept
{
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

static inline int64_t GetPerformanceCounteri386() noexcept
{
    uint64_t r = 0;
    __asm__ volatile ("rdtsc" : "=A"(r)); // Constrain the r variable to the eax:edx pair.
    return r;
}

static inline int64_t GetPerformanceCounterx64() noexcept
{
    uint64_t r1 = 0, r2 = 0;
    __asm__ volatile ("rdtsc" : "=a"(r1), "=d"(r2)); // Constrain r1 to rax and r2 to rdx.
    return (r2 << 32) | r1;
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


BOOST_AUTO_TEST_CASE(cyclecounter_test)
{
    unsigned long long nSample = 100;

    unordered_map<int64_t, unsigned long long> map10msec;
    for (unsigned long long i=0; i< nSample; i++)
    {
        int64_t a = GetPerformanceCounterSysdefault();
        // int64_t a = GetPerformanceCounterHRC();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // sleep for 1 second
        int64_t b = GetPerformanceCounterSysdefault();
        // int64_t b = GetPerformanceCounterHRC();
        map10msec[b-a] += 1;
    }

    for(std::pair<const int64_t, unsigned long long>& cnt: map10msec)
    {
        std::cout << cnt.first << ":" << cnt.second << ",";
    }
    std::cout << std::endl;


}

BOOST_AUTO_TEST_CASE(clock_diff_test)
{
    cout << "Clock difference test" << endl;

    unsigned long long nSample = 1000*1000*1000;

    unordered_map<int64_t, unsigned long long> hrcmap;
    for (unsigned long long i=0; i< nSample; i++)
    {
        int64_t a = GetPerformanceCounterHRC();
        int64_t b = GetPerformanceCounterHRC();
        hrcmap[b-a] += 1;
    }

    cout << "HRC: histogram" << endl;
    for(std::pair<const int64_t, unsigned long long>& cnt: hrcmap)
    {
        std::cout << cnt.first << ":" << cnt.second << ",";
    }
    std::cout << std::endl;

    unordered_map<int64_t, unsigned long long> i386map;
    for (unsigned long long i=0; i< nSample; i++)
    {
        int64_t a = GetPerformanceCounteri386();
        int64_t b = GetPerformanceCounteri386();
        i386map[b-a] += 1;
    }

    cout << "i386: histogram" << endl;
    for(std::pair<const int64_t, unsigned long long>& cnt: i386map)
    {
        std::cout << cnt.first << ":" << cnt.second << ",";
    }
    std::cout << std::endl;


    unordered_map<int64_t, unsigned long long> x64map;
    for (unsigned long long i=0; i< nSample; i++)
    {
        int64_t a = GetPerformanceCounterx64();
        int64_t b = GetPerformanceCounterx64();
        x64map[b-a] += 1;
    }

    cout << "x64: histogram" << endl;
    for(std::pair<const int64_t, unsigned long long>& cnt: x64map)
    {
        std::cout << cnt.first << ":" << cnt.second << ",";
    }
    std::cout << std::endl;


}

BOOST_AUTO_TEST_CASE(clock_benchmark_test)
{
    cout << "Clock benchmark test" << endl;

    unsigned long long nSample = 1000*1000;

#ifdef WIN32
    unordered_map<int64_t, unsigned long long> ftimemap;
    FILETIME ftime1;
    FILETIME ftime2;

    for (unsigned long long i=0; i< nSample; i++)
    {
        GetSystemTimeAsFileTime(&ftime1);
        GetSystemTimeAsFileTime(&ftime2);

        hrcmap[ftime2-ftime1] += 1;
    }
#else
    cout << "gr0000ssssse" << endl;
#endif

//     struct timespec ts = {};
// #    ifdef CLOCK_MONOTONIC
//     clock_gettime(CLOCK_MONOTONIC, &ts);
//     XSW(hasher, ts, "clock_gettime CLOCK_MONOTONIC", "RandAddDynamicEnv");
// #    endif
// #    ifdef CLOCK_REALTIME
//     clock_gettime(CLOCK_REALTIME, &ts);
//     XSW(hasher, ts, "clock_gettime CLOCK_REALTIME", "RandAddDynamicEnv");

// #    endif
// #    ifdef CLOCK_BOOTTIME
//     clock_gettime(CLOCK_BOOTTIME, &ts);
//     XSW(hasher, ts, "clock_gettime CLOCK_BOOTTIME", "RandAddDynamicEnv");
// #    endif
//     // gettimeofday is available on all UNIX systems, but only has microsecond precision.
//     struct timeval tv = {};
//     gettimeofday(&tv, nullptr);
//     XSW(hasher, tv, "gettimeofday", "RandAddDynamicEnv");
// #endif
//     // Probably redundant, but also use all the clocks C++11 provides:
//     XSW(hasher, std::chrono::system_clock::now().time_since_epoch().count(), "std::chrono::system_clock::now().time_since_epoch().count()", "RandAddDynamicEnv");
//     XSW(hasher, std::chrono::steady_clock::now().time_since_epoch().count(), "std::chrono::steady_clock::now().time_since_epoch().count()", "RandAddDynamicEnv");
//     XSW(hasher, std::chrono::high_resolution_clock::now().time_since_epoch().count(), "std::chrono::high_resolution_clock::now().time_since_epoch().count()", "RandAddDynamicEnv");

// #ifndef WIN32
//     // Current resource usage.
//     struct rusage usage = {};
//     if (getrusage(RUSAGE_SELF, &usage) == 0) XSW(hasher, usage, "usage", "RandAddDynamicEnv");
// #endif
}

BOOST_AUTO_TEST_SUITE_END()
