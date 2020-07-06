// Copyright (c) 2014-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CRYPTO_SHA512_H
#define BITCOIN_CRYPTO_SHA512_H

#include <stdint.h>
#include <stdlib.h>

#include <string>
#include <iostream>
using namespace std;

/** A hasher class for SHA-512. */
class CSHA512
{
private:
    uint64_t s[8];
    unsigned char buf[128];
    uint64_t bytes;

    string name;
    int writeCalls = 0;
    int resetCalls = 0;
    int finalizeCalls = 0;
    int instanceNum = -1;


public:
    static constexpr size_t OUTPUT_SIZE = 64;
    CSHA512();
    CSHA512(string new_name);
    CSHA512& Write(const unsigned char* data, size_t len);
    CSHA512& Write(const unsigned char* data, size_t len, string source);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    CSHA512& Reset();
    uint64_t Size() const { return bytes; }
};

#endif // BITCOIN_CRYPTO_SHA512_H
