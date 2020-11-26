// Copyright (c) 2014-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <rng/rngsha512.h>
#include <crypto/sha512.h>

#include <string>

// For PrintThread
#include <iostream>
#include <sstream>
#include <mutex>
#include <memory>

// #include <format>
#include <tinyformat.h>

// For the format PRId64 for int64_t
#include <inttypes.h>

#include <util/time.h>


#include <assert.h>
#include <vector>

#include <sync.h>

#include <fs.h>
#include <serialize.h>
#include <streams.h>
#include <clientversion.h>
#include <rng/rngscribe.h>


class PrintThread: public std::ostringstream
{
public:
    PrintThread() = default;
    ~PrintThread()
    {
        std::lock_guard<std::mutex> guard(_mutexPrint);
        std::cout << this->str();
    }
    private:
        static std::mutex _mutexPrint;
};
std::mutex PrintThread::_mutexPrint{};
CRNGScribe recorder;

// void static pHex(const unsigned char* bytes, size_t size, std::string& out)
// {
//     char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',  'B','C','D','E','F'};
//     for (size_t i = 0; i < size; ++i) {
//         const char ch = bytes[i];
//         out.append(&hex[(ch  & 0xF0) >> 4], 1);
//         out.append(&hex[ch & 0xF], 1);
//     }
// }

Mutex instancectr_lock;
int CSHA512instancectr = 0;
CRNGSHA512::CRNGSHA512(std::string hasherName) : bytes(0)
{
    instancectr_lock.lock();
    instanceNum = ++CSHA512instancectr;
    instancectr_lock.unlock();

    name = hasherName;
}

// CSHA512& CRNGSHA512::Write(const unsigned char* data, size_t len)
// {
//     return hasher.Write(data, len);
// }

CSHA512& CRNGSHA512::Write(const unsigned char* data, size_t len, std::string loc, std::string sourceName)
{
    writeCalls++;
    // RNGRecord dp = RNGRecord(instanceNum, name, loc, sourceName, data, len);
    recorder.AddRecord(instanceNum, name, loc, sourceName, data, len);

    return hasher.Write(data, len);
}

CSHA512& CRNGSHA512::Write(const CEntropySource& source, std::string loc)
{
    assert(source.len >= 0);
    return Write(source.data, source.len, loc, source.name);
}

CSHA512& CRNGSHA512::Write(const ESInt64& source, std::string loc)
{
    return Write((const unsigned char*)&source.data, 64/8, loc, source.name);
}


void CRNGSHA512::Finalize(unsigned char hash[OUTPUT_SIZE], std::string loc)
{
    finalizeCalls++;
    hasher.Finalize(hash);

    // RNGRecord dp = RNGRecord(instanceNum, name, loc, "Finalize", hash, OUTPUT_SIZE);
    recorder.AddRecord(instanceNum, name, loc, "Finalize", hash, OUTPUT_SIZE);
}

CSHA512& CRNGSHA512::Reset(std::string loc)
{
    // RNGRecord dp = RNGRecord(instanceNum, name, loc, "Reset", NULL, 0);
    recorder.AddRecord(instanceNum, name, loc, "Reset", NULL, 0);

    resetCalls++;
    return hasher.Reset();
}
