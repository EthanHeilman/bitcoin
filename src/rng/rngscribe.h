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

// void static pHex(const unsigned char* bytes, size_t size, std::string& out)
// {
//     char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',  'B','C','D','E','F'};
//     for (size_t i = 0; i < size; ++i) {
//         const char ch = bytes[i];
//         out.append(&hex[(ch  & 0xF0) >> 4], 1);
//         out.append(&hex[ch & 0xF], 1);
//     }
// }

struct trecord 
{
    // keys
    int hasherId;
    int locId;
    int srcId;

    // labels
    int dataLen;
    char * data;
};

class RNGRecord
{
public:
    int hasherId = -1;
    std::string hasher;
    std::string loc;
    std::string sourceName;
    char * data;
    size_t len;

RNGRecord(int newHasherId, std::string newHasher, std::string newLoc, std::string newSourceName, 
const unsigned char* newData, size_t newLen)
{
    hasherId = newHasherId;
    hasher = newHasher;
    loc = newLoc;
    sourceName = newSourceName;

    len = newLen;
    data = new char[len];

    std::memcpy(data, newData, len);
}

};


class CRNGScribe
{
private:
    size_t MAX_MEM_SIZE = 5;
    std::vector<RNGRecord>* mem = NULL;
    unsigned int currpart = 0;
    int64_t startTime = GetSystemTimeInSeconds();

    std::vector<RNGRecord> recordsOld;

    std::map<int, std::string> mapHasher;
    std::map<std::string, int> mapLoc;
    int locCtr = 0;
    std::map<std::string, int> mapSrc;
    int srcCtr = 0;
    std::vector<trecord> vRecords;


public:
    // void AddRecord(RNGRecord record)
    // {
    //     recordsOld.push_back(record);
    // }

    void WriteMem(const fs::path& path, std::vector<RNGRecord> *memToWrite)
    {
        FILE *file = fsbridge::fopen(path, "wb");
        CAutoFile fileout(file, SER_DISK, CLIENT_VERSION);
        if (fileout.IsNull()) {
            fileout.fclose();
            //TODO: Handle this better ~ERH
            // PrintThread{}<<"DP: "<<"failed to write file"<<std::endl;
            exit(1);
        }

        // Save the number of datapoints saved
        size_t numDP = memToWrite->size();
        std::cout << "writing out|| " << numDP << std::endl;

        // fileout.write ((const char *) &numDP, sizeof (size_t));
        fileout.write(reinterpret_cast<char const*>(&numDP), sizeof(&numDP));

        for (auto dp :*memToWrite)
        {
            //TODO: Do I need to free each DP? ~ERH
            // std::string hexstr = "";
            // pHex((const unsigned char*)dp.data, dp.len, hexstr);
            fileout<<"..." << dp.hasherId << dp.hasher << dp.loc << dp.sourceName;
            // std::string datastr;
            // pHex((const char*)dp.data, dp.len, &datastr);
            // std::cout << "-->datastr||>" << dp.len << " " << datastr << std::endl;
            fileout.write ((const char *) &dp.len, sizeof (size_t));
            fileout.write(dp.data, dp.len);
        }
        fileout.fclose();
        // ReadMem(path);
    }

    template <typename T>
    auto promote_to_printable_integer_type(T i) -> decltype(+i)
    {
        return +i;
    }

    void ReadMem(const fs::path& path)
    {
        FILE *file = fsbridge::fopen(path, "rb");
        CAutoFile filein(file, SER_DISK, CLIENT_VERSION);
        if (filein.IsNull()) {
            filein.fclose();
            //TODO: Handle this better ~ERH
            std::cout << "Failed to read" << std::endl;
            exit(1);
        }
        std::cout << path << std::endl;

        size_t numDP;
        filein.read((char*)&numDP, 8);
        std::cout<<"numDP||"<<numDP<<std::endl;

        for (size_t i = 0; i < numDP; i++)
        {
            char start[4];
            filein >> start;
            std::cout << "reading||" << start << std::endl;

            int hasherId;
            std::string hasher;
            std::string loc;
            std::string sourceName;
            filein >> hasherId;
            filein >> hasher;
            filein >> loc;
            filein >> sourceName;

            size_t dataLen;
            filein.read((char*)&dataLen, 8);
            std::cout<<"dataLen||"<<dataLen<<std::endl;

            char databuff[dataLen];  
            filein.read((char *)databuff, dataLen);

            std::cout << "-->DP dump||>" << hasherId << " " << hasher << " " << loc << " " << sourceName << std::endl;
            // std::string datastr;
            // pHex(databuff, dataLen);
            // std::cout << "-->datastr||>" << dataLen << " " << datastr << std::endl;

        }

        
        filein.fclose();
        // for (auto dp :*memToWrite)
        // {
        //     //TODO: Do I need to free each DP? ~ERH
        //     fileout<<"DP:"<<dp.hasherId<<","<< dp.hasher <<","<< dp.loc <<","<<dp.sourceName;
        //     fileout.write ((const char *) &dp.len, sizeof (size_t));
        //     fileout.write(dp.data, dp.len);
        // }
        // fileout.fclose();
    }

    Mutex mem_lock;
    size_t AddRecord(int newHasherId, std::string newHasher, std::string newLoc, std::string newSrc, 
const unsigned char* newData, size_t newLen)
    {
        trecord record;

        if (mapHasher.find(newHasherId) == mapHasher.end())
        {
            mapHasher[newHasherId] = newHasher;
        }
        record.hasherId = newHasherId;

        if (mapLoc.find(newLoc) == mapLoc.end())
        {
            mapLoc[newLoc] = locCtr++;
        }
        record.locId = mapLoc[newLoc];

        if (mapSrc.find(newSrc) == mapSrc.end())
        {
            mapSrc[newSrc] = srcCtr++;
        }
        record.srcId = mapSrc[newSrc];

        record.dataLen = newLen;
        record.data = (char *)newData;

        vRecords.push_back(record);

        return vRecords.size();
    }

    /*
    void AddRecord(RNGRecord dp)
    {
        if (mem == NULL) mem = new std::vector<RNGRecord>();

        mem->push_back(dp);


        if (mem->size() > MAX_MEM_SIZE)
        {
            if (mem_lock.try_lock())
            {
                std::vector<RNGRecord> *memToWrite = mem;
                mem = new std::vector<RNGRecord>();
                mem_lock.unlock();

                currpart += 1;
                std::string spath = strprintf("/tmp/rng_%" PRId64 ".%i.dat", this->startTime, currpart);
                // std::string spath = std::format("/tmp/rng_{}_{}.dat", part, time);
                WriteMem(fs::path(spath), memToWrite);
                
                free(memToWrite);
            }

        }
    }
    */

    // void pHex(char* bytes, size_t size)
    // {
    //     char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',  'B','C','D','E','F'};
    //     for (size_t i = 0; i < size; ++i) {
    //         const char ch = bytes[i];
    //         std::cout << &hex[(ch  & 0xF0) >> 4] << &hex[ch & 0xF];
    //     }
    // }

};


#endif // BITCOIN_RNG_TRACER_H