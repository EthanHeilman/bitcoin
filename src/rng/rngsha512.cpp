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
#include <atomic>
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

std::atomic_int CRNGSHA512::instanceCtr{0};
// Mutex instancectr_lock;
CRNGSHA512::CRNGSHA512(std::string hasherType) //: bytes(0)
{
    instanceNum = ++instanceCtr;

    type = hasherType;
}

CRNGSHA512& CRNGSHA512::Write(const unsigned char* data, size_t len, std::string loc, std::string src)
{
    writeCalls++;
    CRNGScribe::GetCRNGScribe().AddRecord(instanceNum, type, loc, src, data, len);

    hasher.Write(data, len);
    return *this;
}

CRNGSHA512& CRNGSHA512::Write(const CEntropySource& source, std::string loc)
{
    assert(source.len >= 0);
    return Write(source.data, source.len, loc, source.name);
}

CRNGSHA512& CRNGSHA512::Write(const ESInt64& source, std::string loc)
{
    return Write((const unsigned char*)&source.data, 64/8, loc, source.name);
}

void CRNGSHA512::Finalize(unsigned char hash[OUTPUT_SIZE], std::string loc)
{
    finalizeCalls++;
    hasher.Finalize(hash);

    CRNGScribe::GetCRNGScribe().AddRecord(instanceNum, type, loc, "Finalize", hash, OUTPUT_SIZE);
}

CRNGSHA512& CRNGSHA512::Reset(std::string loc)
{
    CRNGScribe::GetCRNGScribe().AddRecord(instanceNum, type, loc, "Reset", NULL, 0);

    resetCalls++;
    hasher.Reset();
    return *this;
}
