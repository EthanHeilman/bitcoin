#include <random.h>
#include <random.cpp>
#include <math.h>

#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <random>

#include <iostream>
using namespace std;

BOOST_FIXTURE_TEST_SUITE(fast_tests, BasicTestingSetup)

enum PERFINSTR { sysdefault, hrc, i386, x64 };

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
    vperf.push_back(GetPerformanceCounter());
    do {
        for (int i = 0; i < 1000; ++i) {
            inner_hasher.Finalize(buffer);
            inner_hasher.Reset();
            inner_hasher.Write(buffer, sizeof(buffer));
        }
        // Benchmark operation and feed it into outer hasher.
        int64_t perf = GetPerformanceCounter();
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

static void StrengthenInsti386(const unsigned char (&seed)[32], int microseconds, CSHA512& hasher,
vector<int64_t>& vperf, int& cycle
) noexcept
{
    cycle = 0;
    CSHA512 inner_hasher;
    inner_hasher.Write(seed, sizeof(seed));

    // Hash loop
    unsigned char buffer[64];
    int64_t stop = GetTimeMicros() + microseconds;
    vperf.push_back(GetPerformanceCounteri386());
    do {
        for (int i = 0; i < 1000; ++i) {
            inner_hasher.Finalize(buffer);
            inner_hasher.Reset();
            inner_hasher.Write(buffer, sizeof(buffer));
        }
        // Benchmark operation and feed it into outer hasher.
        int64_t perf = GetPerformanceCounteri386();
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


static void StrengthenInstx64(const unsigned char (&seed)[32], int microseconds, CSHA512& hasher,
vector<int64_t>& vperf, int& cycle
) noexcept
{
    cycle = 0;
    CSHA512 inner_hasher;
    inner_hasher.Write(seed, sizeof(seed));

    // Hash loop
    unsigned char buffer[64];
    int64_t stop = GetTimeMicros() + microseconds;
    vperf.push_back(GetPerformanceCounterx64());
    do {
        for (int i = 0; i < 1000; ++i) {
            inner_hasher.Finalize(buffer);
            inner_hasher.Reset();
            inner_hasher.Write(buffer, sizeof(buffer));
        }
        // Benchmark operation and feed it into outer hasher.
        int64_t perf = GetPerformanceCounterx64();
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
        case PERFINSTR::i386:
        {
            StrengthenInsti386(seed, microseconds, hasher, perfarray, cycle);
            break;
        }
        case PERFINSTR::x64:
        {
            StrengthenInstx64(seed, microseconds, hasher, perfarray, cycle);
            break;
        }
        default:
        {
            assert(0);
        }
    };

}


static void GuessEntropyStrengthen(int microseconds, int nSamples, PERFINSTR instr)
{
    std::unordered_map<int64_t, int> perfsMap;
    int count = 0;
    int minCycles = -1;
    int maxCycles = 0;

    for (int i = 0; i < nSamples; i++)
    {
        vector<int64_t> perfarray;
        int cycle = 0;

        RunStrengthenInst(microseconds, perfarray, cycle, instr);

        for (int j = 1; j < cycle+1; j++)
        {
            int64_t timediff = perfarray[j]-perfarray[j-1];
            perfsMap[timediff] += 1;
            count+=1;
        }

        if (minCycles == -1 || minCycles > cycle) minCycles = cycle;
        if (maxCycles < cycle) maxCycles = cycle;
    }

    vector<pair<int64_t, int>> elems(perfsMap.begin(), perfsMap.end());
    struct {
        bool operator()(std::pair<int64_t, int> a, std::pair<int64_t, int> b) const
        {   
            return a.second > b.second;
        }
    } comp;
    std::sort(elems.begin(), elems.end(), comp);

    int k = 0;
    double expGuess = 0;
    for(pair<int64_t, int> i: elems)
    {
        double prob = (double)i.second/count;
        expGuess += k*prob;
        k+=1;
    }
    cout << "min cycles: " << minCycles << " max cycles: " << maxCycles << " nSamples: " << nSamples << endl; 
    cout <<  "exp guesses: " << expGuess << endl;
    cout <<  "guess entropy (per cycles): " <<  log2(expGuess) << endl;
    cout <<  "guess entropy (total): " <<  log2(expGuess)*minCycles << endl;
}

void PrintSystemParams()
{
    cout << "Defined:";
    #if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
        cout << "#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))" << endl;
    #elif !defined(_MSC_VER) && defined(__i386__)
        cout << "#elif !defined(_MSC_VER) && defined(__i386__)" << endl;
    #elif !defined(_MSC_VER) && (defined(__x86_64__) || defined(__amd64__))
        cout << "#elif !defined(_MSC_VER) && (defined(__x86_64__) || defined(__amd64__))" << endl;
    #else
        cout << "#else" << endl;
    #endif
}


static void GuessEntropyVectorAdd(int nSample)
{
    std::unordered_map<int64_t, int> perfsMap;
    vector<int64_t> vperf;
    int cycle = 0;

    for (int i = 0; i < nSample; ++i) {
        // Benchmark operation and feed it into outer hasher.
        int64_t perf = GetPerformanceCounter();
        //ERH -- Added code
        vperf.push_back(perf);
        cycle++;
    }

    for (int j = 1; j < nSample; j++)
    {
        int64_t timediff = vperf[j]-vperf[j-1];
        perfsMap[timediff] += 1;
    }

    vector<pair<int64_t, int>> elems(perfsMap.begin(), perfsMap.end());
    struct {
        bool operator()(std::pair<int64_t, int> a, std::pair<int64_t, int> b) const
        {   
            return a.second > b.second;
        }   
    } comp;
    std::sort(elems.begin(), elems.end(), comp);


    int k = 0;
    double expGuess = 0;
    for(pair<int64_t, int> i: elems)
    {
        double prob = (double)i.second/nSample;
        expGuess += k*prob;
        k+=1;
    }
    cout << "per cycle (skipping the first cycle)" << endl;
    cout <<  "exp guesses (guessing entropy): " << expGuess << endl;
    cout <<  "bits of guess entropy: " <<  log2(expGuess) << endl;
}

// static void SeedFast(CSHA512& hasher) noexcept
// {
//     unsigned char buffer[32];

//     // Stack pointer to indirectly commit to thread/callstack
//     const unsigned char* ptr = buffer;
//     hasher.Write((const unsigned char*)&ptr, sizeof(ptr));

//     // Hardware randomness is very fast when available; use it always.
//     SeedHardwareFast(hasher);

//     // High-precision timestamp
//     SeedTimestamp(hasher);
// }


BOOST_AUTO_TEST_CASE(guessing_ent_fast_test)
{
    int nSample = 5000;
    
    cout << "Fast System Parameters" << endl;
    cout << "========" << endl;
    PrintSystemParams();
    cout << endl;

    // cout << "Measuring Strengthen: " << "System Default" << endl;
    // cout << "========" << endl;
    // cout << "microseconds: " << microseconds << endl;
    // cout << "num samples: " << nSample << endl;
    // GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::sysdefault);
    // cout << endl;

    // cout << "Measuring Strengthen: " << "HRC" << endl;
    // cout << "========" << endl;
    // cout << "microseconds: " << microseconds << endl;
    // cout << "num samples: " << nSample << endl;
    // GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::hrc);
    // cout << endl;

    // cout << "Measuring Strengthen: " << "i386" << endl;
    // cout << "========" << endl;
    // cout << "microseconds: " << microseconds << endl;
    // cout << "num samples: " << nSample << endl;
    // GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::i386);
    // cout << endl;

    // cout << "Measuring Strengthen: " << "x64" << endl;
    // cout << "========" << endl;
    // cout << "microseconds: " << microseconds << endl;
    // cout << "num samples: " << nSample << endl;
    // GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::x64);
    // cout << endl;


    // cout << "Measuring vector add" << endl;
    // cout << "========" << endl;
    // cout << "num samples: " << nSample << endl;
    // GuessEntropyVectorAdd(nSample);
}


// static void countSamples(vector<int> vSample)
// {
//     std::unordered_map<int, int> sampleMap;

//     for (int sample: vSample){
//         // cout << "SAAAMPLE " << sample << endl;
//         sampleMap[sample] += 1;
//     }
//     vector<pair<int, int>> counts(sampleMap.begin(), sampleMap.end());
//     struct {
//         bool operator()(std::pair<int64_t, int> a, std::pair<int64_t, int> b) const
//         {   
//             return a.second > b.second;
//         }
//     } comp;
//     std::sort(counts.begin(), counts.end(), comp);
 
//     pair<int, int> front = counts.front();
//     cout << "count " << front.first << " " << front.second << " exp " << vSample.size()/counts.size() << " diff " << front.second - vSample.size()/counts.size() << endl;
//     pair<int, int> back = counts.back();
//     cout << "count " << back.first << " " << back.second << " exp " << vSample.size()/counts.size() << " diff " <<  vSample.size()/counts.size() - back.second << endl;
// }

// BOOST_AUTO_TEST_CASE(randrange_test)
// {
//     FastRandomContext insecure_rand;
//     int nSamples = 500000000;

//     int runs = 10;
//     for (int r = 0; r < runs; r++)
//     {
//         for (int r = 3; r < 4; r++)
//         {
//             vector<int> vSamples;
//             for (int i = 0; i < nSamples; i++)
//             {
//                 int sample = insecure_rand.randrange(r);
//                 vSamples.push_back(sample);
//             }
//             cout << "r= " << r << endl;
//             countSamples(vSamples);
//         }
//     }
// }

}