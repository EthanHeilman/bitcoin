// Copyright (c) 2014-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RNG_SHA512_H
#define BITCOIN_RNG_SHA512_H

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <crypto/sha512.h>

#include <rng/entropysource.h>

/** A hasher class for SHA-512. */
class CRNGSHA512
{

private:
    static unsigned int instanceCtr;

    uint64_t bytes;
    std::string name;
    unsigned int writeCalls = 0;
    unsigned int resetCalls = 0;
    unsigned int finalizeCalls = 0;
    unsigned int instanceNum = 0;

    CSHA512 hasher;

public:
    static constexpr size_t OUTPUT_SIZE = 64;

    CRNGSHA512(std::string hasherName);
    // CSHA512& Write(const unsigned char* data, size_t len);
    CSHA512& Write(const unsigned char* data, size_t len, std::string loc, std::string sourceName);
    CSHA512& Write(const CEntropySource& source, std::string loc);
    CSHA512& Write(const ESInt64& source, std::string loc);
    void Finalize(unsigned char hash[OUTPUT_SIZE], std::string loc);
    CSHA512& Reset(std::string loc);
    uint64_t Size() const { return bytes; }
};


#endif // BITCOIN_RNG_SHA512_H