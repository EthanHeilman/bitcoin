// Copyright (c) 2014-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RNG_SHA256_H
#define BITCOIN_RNG_SHA256_H

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <crypto/sha256.h>

#include <rng/entropysource.h>
#include <rng/rngscribe.h>


/** A hasher class for SHA-512. */
class CRNGSHA256
{

private:
    static unsigned int instanceCtr;

    uint64_t bytes;
    std::string type;
    unsigned int writeCalls = 0;
    unsigned int resetCalls = 0;
    unsigned int finalizeCalls = 0;
    unsigned int instanceNum = 0;

    CSHA256 hasher;

public:
    static constexpr size_t OUTPUT_SIZE = 32;

    CRNGSHA256(std::string hasherType);
    // CSHA512& Write(const unsigned char* data, size_t len);
    CRNGSHA256& Write(const unsigned char* data, size_t len, std::string loc, std::string src);
    CRNGSHA256& Write(const CEntropySource& source, std::string loc);
    CRNGSHA256& Write(const ESInt32& source, std::string loc);
    CRNGSHA256& Write(const ESInt64& source, std::string loc);
    void Finalize(unsigned char hash[OUTPUT_SIZE], std::string loc);
    CRNGSHA256& Reset(std::string loc);
    uint64_t Size() const { return bytes; }
};


#endif // BITCOIN_RNG_SHA256_H