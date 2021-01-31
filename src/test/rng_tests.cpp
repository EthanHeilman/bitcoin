// #include <random.h>
// #include <random.cpp>
// #include <math.h>

// #include <test/util/setup_common.h>

// #include <boost/test/unit_test.hpp>

// #include <algorithm>
// #include <random>

// #include <iostream>
// using namespace std;

// BOOST_FIXTURE_TEST_SUITE(rng_tests, BasicTestingSetup)

// void pHex(unsigned char* bytes, int size)
// {
//     char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',   'B','C','D','E','F'};
//     std::string str;
//     for (int i = 0; i < size; ++i) {
//         const char ch = bytes[i];
//         str.append(&hex[(ch  & 0xF0) >> 4], 1);
//         str.append(&hex[ch & 0xF], 1);
//     }
//     cout << str <<endl;
// }

// bool PrintDiffOn = true;

// enum PERFINSTR { sysdefault, hrc, i386, x64 };

// static inline int64_t GetPerformanceCounterHRC() noexcept
// {
//     return std::chrono::high_resolution_clock::now().time_since_epoch().count();
// }

// static inline int64_t GetPerformanceCounteri386() noexcept
// {
//     uint64_t r = 0;
//     __asm__ volatile ("rdtsc" : "=A"(r)); // Constrain the r variable to the eax:edx pair.
//     return r;
// }

// static inline int64_t GetPerformanceCounterx64() noexcept
// {
//     uint64_t r1 = 0, r2 = 0;
//     __asm__ volatile ("rdtsc" : "=a"(r1), "=d"(r2)); // Constrain r1 to rax and r2 to rdx.
//     return (r2 << 32) | r1;
// }

// static inline int64_t GetPerformanceCounterSysdefault() noexcept
// {
//     // Read the hardware time stamp counter when available.
//     // See https://en.wikipedia.org/wiki/Time_Stamp_Counter for more information.
//     #if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
//         return __rdtsc();
//     #elif !defined(_MSC_VER) && defined(__i386__)
//         uint64_t r = 0;
//         __asm__ volatile ("rdtsc" : "=A"(r)); // Constrain the r variable to the eax:edx pair.
//         return r;
//     #elif !defined(_MSC_VER) && (defined(__x86_64__) || defined(__amd64__))
//         uint64_t r1 = 0, r2 = 0;
//         __asm__ volatile ("rdtsc" : "=a"(r1), "=d"(r2)); // Constrain r1 to rax and r2 to rdx.
//         return (r2 << 32) | r1;
//     #else
//         // Fall back to using C++11 clock (usually microsecond or nanosecond precision)
//         return std::chrono::high_resolution_clock::now().time_since_epoch().count();
//     #endif
// }

// // Instrumented version of strengthen to capture:
// // perf per cycle (loop)
// // number of cycles
// static void StrengthenInst(const unsigned char (&seed)[32], int microseconds, CSHA512& hasher,
// vector<int64_t>& vperf, int& cycle
// ) noexcept
// {
//     cycle = 0;
//     CSHA512 inner_hasher;
//     inner_hasher.Write(seed, sizeof(seed));

//     // Hash loop
//     unsigned char buffer[64];
//     int64_t stop = GetTimeMicros() + microseconds;
//     vperf.push_back(GetPerformanceCounter());
//     do {
//         for (int i = 0; i < 1000; ++i) {
//             inner_hasher.Finalize(buffer);
//             inner_hasher.Reset();
//             inner_hasher.Write(buffer, sizeof(buffer));
//         }
//         // Benchmark operation and feed it into outer hasher.
//         int64_t perf = GetPerformanceCounter();
//         //ERH -- Added code
//         vperf.push_back(perf);
//         cycle++;
//         // -- 
//         hasher.Write((const unsigned char*)&perf, sizeof(perf));
//     } while (GetTimeMicros() < stop);

//     // Produce output from inner state and feed it to outer hasher.
//     inner_hasher.Finalize(buffer);
//     hasher.Write(buffer, sizeof(buffer));
//     // Try to clean up.
//     inner_hasher.Reset();
//     memory_cleanse(buffer, sizeof(buffer));
// }

// static void StrengthenInstHRC(const unsigned char (&seed)[32], int microseconds, CSHA512& hasher,
// vector<int64_t>& vperf, int& cycle
// ) noexcept
// {
//     cycle = 0;
//     CSHA512 inner_hasher;
//     inner_hasher.Write(seed, sizeof(seed));

//     // Hash loop
//     unsigned char buffer[64];
//     int64_t stop = GetTimeMicros() + microseconds;
//     vperf.push_back(GetPerformanceCounterHRC());
//     do {
//         for (int i = 0; i < 1000; ++i) {
//             inner_hasher.Finalize(buffer);
//             inner_hasher.Reset();
//             inner_hasher.Write(buffer, sizeof(buffer));
//         }
//         // Benchmark operation and feed it into outer hasher.
//         int64_t perf = GetPerformanceCounterHRC();
//         //ERH -- Added code
//         vperf.push_back(perf);
//         cycle++;
//         // -- 
//         hasher.Write((const unsigned char*)&perf, sizeof(perf));
//     } while (GetTimeMicros() < stop);

//     // Produce output from inner state and feed it to outer hasher.
//     inner_hasher.Finalize(buffer);
//     hasher.Write(buffer, sizeof(buffer));
//     // Try to clean up.
//     inner_hasher.Reset();
//     memory_cleanse(buffer, sizeof(buffer));
// }

// static void StrengthenInsti386(const unsigned char (&seed)[32], int microseconds, CSHA512& hasher,
// vector<int64_t>& vperf, int& cycle
// ) noexcept
// {
//     cycle = 0;
//     CSHA512 inner_hasher;
//     inner_hasher.Write(seed, sizeof(seed));

//     // Hash loop
//     unsigned char buffer[64];
//     int64_t stop = GetTimeMicros() + microseconds;
//     vperf.push_back(GetPerformanceCounteri386());
//     do {
//         for (int i = 0; i < 1000; ++i) {
//             inner_hasher.Finalize(buffer);
//             inner_hasher.Reset();
//             inner_hasher.Write(buffer, sizeof(buffer));
//         }
//         // Benchmark operation and feed it into outer hasher.
//         int64_t perf = GetPerformanceCounteri386();
//         //ERH -- Added code
//         vperf.push_back(perf);
//         cycle++;
//         // -- 
//         hasher.Write((const unsigned char*)&perf, sizeof(perf));
//     } while (GetTimeMicros() < stop);

//     // Produce output from inner state and feed it to outer hasher.
//     inner_hasher.Finalize(buffer);
//     hasher.Write(buffer, sizeof(buffer));
//     // Try to clean up.
//     inner_hasher.Reset();
//     memory_cleanse(buffer, sizeof(buffer));
// }


// static void StrengthenInstx64(const unsigned char (&seed)[32], int microseconds, CSHA512& hasher,
// vector<int64_t>& vperf, int& cycle
// ) noexcept
// {
//     cycle = 0;
//     CSHA512 inner_hasher;
//     inner_hasher.Write(seed, sizeof(seed));

//     // Hash loop
//     unsigned char buffer[64];
//     int64_t stop = GetTimeMicros() + microseconds;
//     vperf.push_back(GetPerformanceCounterx64());
//     do {
//         for (int i = 0; i < 1000; ++i) {
//             inner_hasher.Finalize(buffer);
//             inner_hasher.Reset();
//             inner_hasher.Write(buffer, sizeof(buffer));
//         }
//         // Benchmark operation and feed it into outer hasher.
//         int64_t perf = GetPerformanceCounterx64();
//         //ERH -- Added code
//         vperf.push_back(perf);
//         cycle++;
//         // -- 
//         hasher.Write((const unsigned char*)&perf, sizeof(perf));
//     } while (GetTimeMicros() < stop);

//     // Produce output from inner state and feed it to outer hasher.
//     inner_hasher.Finalize(buffer);
//     hasher.Write(buffer, sizeof(buffer));
//     // Try to clean up.
//     inner_hasher.Reset();
//     memory_cleanse(buffer, sizeof(buffer));
// }


// static void RunStrengthenInst(int microseconds, vector<int64_t>& perfarray, int& cycle, PERFINSTR instr)
// {
//     unsigned char seed[32];
//     GetRandBytes(seed, 32);
//     CSHA512 hasher;

//     switch (instr)
//     {
//         case PERFINSTR::sysdefault:
//         {
//             StrengthenInst(seed, microseconds, hasher, perfarray, cycle);
//             break;
//         }
//         case PERFINSTR::hrc:
//         {
//             StrengthenInstHRC(seed, microseconds, hasher, perfarray, cycle);
//             break;
//         }
//         case PERFINSTR::i386:
//         {
//             StrengthenInsti386(seed, microseconds, hasher, perfarray, cycle);
//             break;
//         }
//         case PERFINSTR::x64:
//         {
//             StrengthenInstx64(seed, microseconds, hasher, perfarray, cycle);
//             break;
//         }
//         default:
//         {
//             assert(0);
//         }
//     };

// }


// static void GuessEntropyStrengthen(int microseconds, int nSamples, PERFINSTR instr)
// {
//     std::unordered_map<int64_t, int> perfsMap;
//     int count = 0;
//     int minCycles = -1;
//     int maxCycles = 0;

//     for (int i = 0; i < nSamples; i++)
//     {
//         vector<int64_t> perfarray;
//         int cycle = 0;

//         RunStrengthenInst(microseconds, perfarray, cycle, instr);

//         for (int64_t j = 1; j < cycle+1; j++)
//         {
//             int64_t timediff = perfarray[j]-perfarray[j-1];
//             perfsMap[timediff] += 1;
//             count+=1;
//         }

//         if (minCycles == -1 || minCycles > cycle) minCycles = cycle;
//         if (maxCycles < cycle) maxCycles = cycle;
//     }

//     vector<pair<int64_t, int>> elems(perfsMap.begin(), perfsMap.end());
//     struct {
//         bool operator()(std::pair<int64_t, int> a, std::pair<int64_t, int> b) const
//         {   
//             return a.second > b.second;
//         }
//     } comp;
//     std::sort(elems.begin(), elems.end(), comp);

//     int k = 0;
//     double expGuess = 0;
//     for(pair<int64_t, int> i: elems)
//     {
//         double prob = (double)i.second/count;
//         expGuess += k*prob;
//         k+=1;
//     }
//     cout << "min cycles: " << minCycles << " max cycles: " << maxCycles << " nSamples: " << nSamples << endl; 
//     cout <<  "exp guesses: " << expGuess << endl;
//     cout <<  "guess entropy (per cycles): " <<  log2(expGuess) << endl;
//     cout <<  "guess entropy (total): " <<  log2(expGuess)*minCycles << endl;
// }

// void PrintSystemParams()
// {
//     cout << "Defined:";
//     #if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
//         cout << "#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))" << endl;
//     #elif !defined(_MSC_VER) && defined(__i386__)
//         cout << "#elif !defined(_MSC_VER) && defined(__i386__)" << endl;
//     #elif !defined(_MSC_VER) && (defined(__x86_64__) || defined(__amd64__))
//         cout << "#elif !defined(_MSC_VER) && (defined(__x86_64__) || defined(__amd64__))" << endl;
//     #else
//         cout << "#else" << endl;
//     #endif
// }

// static void PrintDiffs(vector<pair<int64_t, int>> elemsIn)
// {
//     if (!PrintDiffOn) return;
//     // sort the input map by the first value not the second
//     vector<pair<int64_t, int>> elems(elemsIn.begin(), elemsIn.end());
//     struct {
//         bool operator()(std::pair<int64_t, int> a, std::pair<int64_t, int> b) const
//         {   
//             return a.first < b.first;
//         }   
//     } comp;
//     std::sort(elems.begin(), elems.end(), comp);

//     cout << "Diffs: " << endl;

//     std::vector<double> buckets;
//     for(pair<int64_t, int> elem :elems)
//     {
//         cout<<"diff:"<<elem.first<<","<<elem.second<<endl;
//     }
// }

// static void ComputeGeEnt(vector<int64_t>& vperf)
// {
//     std::unordered_map<int64_t, int> perfsMap;
//     for (uint64_t j = 1; j < vperf.size(); j++)
//     {
//         int64_t timediff = vperf[j]-vperf[j-1];
//         perfsMap[timediff] += 1;
//     }

//     vector<pair<int64_t, int>> elems(perfsMap.begin(), perfsMap.end());
//     struct {
//         bool operator()(std::pair<int64_t, int> a, std::pair<int64_t, int> b) const
//         {   
//             return a.second > b.second;
//         }   
//     } comp;
//     std::sort(elems.begin(), elems.end(), comp);

//     int k = 0;
//     double expGuess = 0;
//     for(pair<int64_t, int> i: elems)
//     {
//         double prob = (double)i.second/vperf.size();
//         expGuess += k*prob;
//         k+=1;
//         // if (prob >0.01) cout << i.first << " " << prob << endl;
//     }
//     cout << "exp guesses (guessing entropy): " << expGuess << endl;
//     cout << "bits of guess entropy: " <<  log2(expGuess) << endl;
//     cout << "vperf.size(): " <<  vperf.size() << endl;
//     PrintDiffs(elems);
// }

// static void ComputeGeEntPair(vector<int64_t>& vperf)
// {
//     std::unordered_map<int64_t, int> perfsMap;
//     for (unsigned long j = 1; j < vperf.size()-1; j+=2)
//     {
//         int64_t timediff = vperf[j]-vperf[j-1];
//         perfsMap[timediff] += 1;
//     }
//     vector<pair<int64_t, int>> elems(perfsMap.begin(), perfsMap.end());
//     struct {
//         bool operator()(std::pair<int64_t, int> a, std::pair<int64_t, int> b) const
//         {   
//             return a.second > b.second;
//         }   
//     } comp;

//     std::sort(elems.begin(), elems.end(), comp);

//     int k = 0;
//     double expGuess = 0;
//     for(pair<int64_t, int> i: elems)
//     {
//         double prob = 2*(double)i.second/elems.size();
//         expGuess += k*prob;
//         k+=1;
//         if (prob >0.01) cout << i.first << " " << prob << endl;
//     }
//     cout << "Within a call to SLOW" << endl;
//     cout << "exp guesses (guessing entropy): " << expGuess << endl;
//     cout << "bits of guess entropy: " <<  log2(expGuess) << endl;
//     cout << "vperf.size(): " <<  vperf.size() << endl;
//     PrintDiffs(elems);
// }

// static void ComputeGeEntPairMinusOne(vector<int64_t>& vperf)
// {
//     std::unordered_map<int64_t, int> perfsMap;
//     for (uint64_t j = 2; j < vperf.size()-1; j+=2)
//     {
//         int64_t timediff = vperf[j]-vperf[j-1];
//         perfsMap[timediff] += 1;
//     }
//     vector<pair<int64_t, int>> elems(perfsMap.begin(), perfsMap.end());
//     struct {
//         bool operator()(std::pair<int64_t, int> a, std::pair<int64_t, int> b) const
//         {   
//             return a.second > b.second;
//         }   
//     } comp;

//     std::sort(elems.begin(), elems.end(), comp);

//     int k = 0;
//     double expGuess = 0;
//     for(pair<int64_t, int> i: elems)
//     {
//         double prob = 2*(double)i.second/(elems.size()-1);
//         expGuess += k*prob;
//         k+=1;
//         if (prob >0.01) cout << i.first << " " << prob << endl;
//     }
//     cout << "Between calls to SLOW" << endl;
//     cout << "exp guesses (guessing entropy): " << expGuess << endl;
//     cout << "bits of guess entropy: " <<  log2(expGuess) << endl;
//     cout << "vperf.size(): " <<  vperf.size() << endl;
//     PrintDiffs(elems);
// }

// static void GuessEntropyVectorAdd(int nSample)
// {
//     std::unordered_map<int64_t, int> perfsMap;
//     vector<int64_t> vperf;
//     int cycle = 0;

//     for (int i = 0; i < nSample; ++i) {
//         // Benchmark operation and feed it into outer hasher.
//         int64_t perf = GetPerformanceCounter();
//         //ERH -- Added code
//         vperf.push_back(perf);
//         cycle++;
//     }
//     ComputeGeEnt(vperf);
// }


// BOOST_AUTO_TEST_CASE(guessing_ent_10ms_test)
// {
//     int microseconds = 10*1000;
//     int nSample = 5000;
    
//     cout << "Strenthen System Parameters" << endl;
//     cout << "========" << endl;
//     PrintSystemParams();
//     cout << endl;

//     cout << "Measuring Strengthen: " << "System Default" << endl;
//     cout << "========" << endl;
//     cout << "microseconds: " << microseconds << endl;
//     cout << "num samples: " << nSample << endl;
//     GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::sysdefault);
//     cout << endl;

//     cout << "Measuring Strengthen: " << "HRC" << endl;
//     cout << "========" << endl;
//     cout << "microseconds: " << microseconds << endl;
//     cout << "num samples: " << nSample << endl;
//     GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::hrc);
//     cout << endl;

//     cout << "Measuring Strengthen: " << "i386" << endl;
//     cout << "========" << endl;
//     cout << "microseconds: " << microseconds << endl;
//     cout << "num samples: " << nSample << endl;
//     GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::i386);
//     cout << endl;

//     cout << "Measuring Strengthen: " << "x64" << endl;
//     cout << "========" << endl;
//     cout << "microseconds: " << microseconds << endl;
//     cout << "num samples: " << nSample << endl;
//     GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::x64);
//     cout << endl;

//     cout << "Measuring vector add" << endl;
//     cout << "========" << endl;
//     cout << "num samples: " << nSample << endl;
//     GuessEntropyVectorAdd(nSample);
// }

// BOOST_AUTO_TEST_CASE(guessing_ent_100ms_test)
// {
//     int microseconds = 100*1000;
//     int nSample = 5000;

//     cout << "Strenthen System Parameters" << endl;
//     cout << "========" << endl;
//     PrintSystemParams();
//     cout << endl;

//     cout << "Measuring Strengthen: " << "System Default" << endl;
//     cout << "========" << endl;
//     cout << "microseconds: " << microseconds << endl;
//     cout << "num samples: " << nSample << endl;
//     GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::sysdefault);
//     cout << endl;

//     cout << "Measuring Strengthen: " << "HRC" << endl;
//     cout << "========" << endl;
//     cout << "microseconds: " << microseconds << endl;
//     cout << "num samples: " << nSample << endl;
//     GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::hrc);
//     cout << endl;

//     cout << "Measuring Strengthen: " << "i386" << endl;
//     cout << "========" << endl;
//     cout << "microseconds: " << microseconds << endl;
//     cout << "num samples: " << nSample << endl;
//     GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::i386);
//     cout << endl;

//     cout << "Measuring Strengthen: " << "x64" << endl;
//     cout << "========" << endl;
//     cout << "microseconds: " << microseconds << endl;
//     cout << "num samples: " << nSample << endl;
//     GuessEntropyStrengthen(microseconds, nSample, PERFINSTR::x64);
//     cout << endl;

//     cout << "Measuring vector add" << endl;
//     cout << "========" << endl;
//     cout << "num samples: " << nSample << endl;
//     GuessEntropyVectorAdd(nSample);
// }


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

// static int64_t SeedTimestampTest(CSHA512& hasher, PERFINSTR instr) noexcept
// {

//     switch (instr)
//     {
//         case PERFINSTR::sysdefault:
//         {
//             int64_t perfcounter = GetPerformanceCounterSysdefault();
//             hasher.Write((const unsigned char*)&perfcounter, sizeof(perfcounter));
//             return perfcounter;
//         }
//         case PERFINSTR::hrc:
//         {
//             int64_t perfcounter = GetPerformanceCounterHRC();
//             hasher.Write((const unsigned char*)&perfcounter, sizeof(perfcounter));
//             return perfcounter;
//         }
//         case PERFINSTR::i386:
//         {
//             int64_t perfcounter = GetPerformanceCounteri386();
//             hasher.Write((const unsigned char*)&perfcounter, sizeof(perfcounter));
//             return perfcounter;
//         }
//         case PERFINSTR::x64:
//         {
//             int64_t perfcounter = GetPerformanceCounterx64();
//             hasher.Write((const unsigned char*)&perfcounter, sizeof(perfcounter));
//             return perfcounter;
//         }
//         default:
//         {
//             assert(0);
//         }
//     };
// }

// static void SeedFastTest(CSHA512& hasher, vector<int64_t>& vperf, PERFINSTR instr) noexcept
// {
//     unsigned char buffer[32];

//     // Stack pointer to indirectly commit to thread/callstack
//     const unsigned char* ptr = buffer;
//     hasher.Write((const unsigned char*)&ptr, sizeof(ptr));

//     // Hardware randomness is very fast when available; use it always.
//     SeedHardwareFast(hasher);

//     // High-precision timestamp
//     int64_t perfcounter = SeedTimestampTest(hasher, instr);
//     vperf.push_back(perfcounter);
// }

// static void SeedSlowTest(CSHA512& hasher, RNGState& rng, vector<int64_t>& vperf, PERFINSTR instr) noexcept
// {
//     unsigned char buffer[32];

//     // Everything that the 'fast' seeder includes
//     SeedFastTest(hasher, vperf, instr);

//     // OS randomness
//     GetOSRand(buffer);
//     hasher.Write(buffer, sizeof(buffer));

//     // Add the events hasher into the mix
//     rng.SeedEvents(hasher);

//     // High-precision timestamp.
//     //
//     // Note that we also commit to a timestamp in the Fast seeder, so we indirectly commit to a
//     // benchmark of all the entropy gathering sources in this function).
//     int64_t perfcounter = SeedTimestampTest(hasher, instr);
//     vperf.push_back(perfcounter);
// }

// static void ProcRandTest(unsigned char* out, int num, RNGLevel level, vector<int64_t>& vperf, PERFINSTR instr) noexcept
// {
//     // Make sure the RNG is initialized first (as all Seed* function possibly need hwrand to be available).
//     RNGState& rng = GetRNGState();

//     assert(num <= 32);

//     CSHA512 hasher;
//     switch (level) {
//     case RNGLevel::FAST:
//         SeedFastTest(hasher, vperf, instr);
//         break;
//     case RNGLevel::SLOW:
//         SeedSlowTest(hasher, rng, vperf, instr);
//         break;
//     case RNGLevel::PERIODIC:
//         SeedPeriodic(hasher, rng);
//         break;
//     }

//     // Combine with and update state
//     if (!rng.MixExtract(out, num, std::move(hasher), false)) {
//         assert(0); // Fail if we end up in this path
//         // On the first invocation, also seed with SeedStartup().
//         CSHA512 startup_hasher;
//         SeedStartup(startup_hasher, rng);
//         rng.MixExtract(out, num, std::move(startup_hasher), true);
//     }
// }

// static void RunProcRandTest(int nSample, RNGLevel level, PERFINSTR instr)
// {
//     RandomInit();

//     unsigned char buf[32];
//     int num = 32;
//     vector<int64_t> vperf;

//     for (int i = 0; i < nSample; i++)
//     {
//         ProcRandTest(buf, num, level, vperf, instr);
//     }
//     switch (level) {
//     case RNGLevel::FAST:
//         ComputeGeEnt(vperf);
//         break;
//     case RNGLevel::SLOW:
//         ComputeGeEntPair(vperf);
//         cout << endl;
//         ComputeGeEntPairMinusOne(vperf);
//         break;
//     default:
//         assert(0);
//     }
// }

// BOOST_AUTO_TEST_CASE(guessing_ent_proc_rand_test)
// {
//     int nSample = 50000;
    
//     cout << "ProcRand System Parameters" << endl;
//     cout << "========" << endl;
//     PrintSystemParams();
//     cout << endl;
    
//     cout << "Measuring ProcRand Fast: " << "System Default" << endl;
//     cout << "========" << endl;
//     cout << "num samples: " << nSample << endl;
//     RunProcRandTest(nSample, RNGLevel::FAST, PERFINSTR::sysdefault);
//     cout << endl;

//     cout << "Measuring ProcRand Fast: " << "HRC" << endl;
//     cout << "========" << endl;
//     cout << "num samples: " << nSample << endl;
//     RunProcRandTest(nSample, RNGLevel::FAST, PERFINSTR::hrc);
//     cout << endl;
    
//     cout << "Measuring ProcRand Fast: " << "i386" << endl;
//     cout << "========" << endl;
//     cout << "num samples: " << nSample << endl;
//     RunProcRandTest(nSample, RNGLevel::FAST, PERFINSTR::i386);
//     cout << endl;

//     cout << "Measuring ProcRand Fast: " << "x64" << endl;
//     cout << "========" << endl;
//     cout << "num samples: " << nSample << endl;
//     RunProcRandTest(nSample, RNGLevel::FAST, PERFINSTR::x64);
//     cout << endl;

//     cout << "Measuring ProcRand Slow: " << "System Default" << endl;
//     cout << "========" << endl;
//     cout << "num samples: " << nSample << endl;
//     RunProcRandTest(nSample, RNGLevel::SLOW, PERFINSTR::sysdefault);
//     cout << endl;

//     cout << "Measuring ProcRand Slow: " << "HRC" << endl;
//     cout << "========" << endl;
//     cout << "num samples: " << nSample << endl;
//     RunProcRandTest(nSample, RNGLevel::SLOW, PERFINSTR::hrc);
//     cout << endl;

//     cout << "Measuring ProcRand Slow: " << "i386" << endl;
//     cout << "========" << endl;
//     cout << "num samples: " << nSample << endl;
//     RunProcRandTest(nSample, RNGLevel::SLOW, PERFINSTR::i386);
//     cout << endl;

//     cout << "Measuring ProcRand Slow: " << "x64" << endl;
//     cout << "========" << endl;
//     cout << "num samples: " << nSample << endl;
//     RunProcRandTest(nSample, RNGLevel::SLOW, PERFINSTR::x64);
//     cout << endl;



//     cout << "Measuring vector add" << endl;
//     cout << "========" << endl;
//     cout << "num samples: " << nSample << endl;
//     GuessEntropyVectorAdd(nSample);
// }



// }