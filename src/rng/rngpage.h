// Copyright (c) 2014-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RNG_BOOK_H
#define BITCOIN_RNG_BOOK_H

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <crypto/sha512.h>

#include <rng/entropysource.h>

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


class CRNGPage
{
private:

    struct trecord 
    {
        // keys
        unsigned int hasherId;
        unsigned int hasherType;
        unsigned int locId;
        unsigned int srcId;

        // labels
        unsigned int len;
        char * data;
    };

public:
    // std::map<unsigned int, std::string> mapHasher;
    std::map<std::string, unsigned int> mapType;
    int typeCtr = 0;
    std::map<std::string, unsigned int> mapLoc;
    int locCtr = 0;
    std::map<std::string, unsigned int> mapSrc;
    int srcCtr = 0;

    Mutex vRecords_lock;
    std::vector<trecord> vRecords;

    ~CRNGPage();

    void Write(fs::path path);

    template <typename Stream>
    void Serialize(Stream& s) const;

    template <typename Stream>
    void Unserialize(Stream& s);

    size_t AddRecord(unsigned int newHasherId, std::string newHasher, std::string newLoc, std::string newSrc, 
const unsigned char* newData, size_t len);
};


#endif // BITCOIN_RNG_BOOK_H