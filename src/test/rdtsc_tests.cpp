#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <random>

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

BOOST_AUTO_TEST_SUITE_END()
