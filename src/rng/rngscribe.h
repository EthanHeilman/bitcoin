// Copyright (c) 2014-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RNG_TRACER_H
#define BITCOIN_RNG_TRACER_H

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <crypto/sha512.h>

#include <rng/entropysource.h>
#include <rng/rngpage.h>

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
#include <support/allocators/secure.h>

class CRNGScribe
{

public:
    size_t MAX_MEM_SIZE = 50000;
    int64_t startTime = GetSystemTimeInSeconds();
    unsigned int currpart = 0;
    CRNGPage* currPage;

    // Using singleton pattern here but allow constructor to be calls for testing purposes
    CRNGScribe();

    Mutex mem_lock;
    void AddRecord(unsigned int hasherId, std::string type, std::string loc, std::string src, 
const unsigned char* data, size_t len);
    void WritePage();

    static CRNGScribe& GetCRNGScribe() noexcept;

};

#endif // BITCOIN_RNG_TRACER_H