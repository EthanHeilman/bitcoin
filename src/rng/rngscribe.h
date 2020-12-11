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
    unsigned int hasherId;
    unsigned int hasherType;
    unsigned int locId;
    unsigned int srcId;

    // labels
    unsigned int len;
    char * data;
};


class CRNGCodex
{
private:
    int64_t startTime = GetSystemTimeInSeconds();

public:
    // std::map<unsigned int, std::string> mapHasher;
    std::map<std::string, unsigned int> mapType;
    int typeCtr = 0;
    std::map<std::string, unsigned int> mapLoc;
    int locCtr = 0;
    std::map<std::string, unsigned int> mapSrc;
    int srcCtr = 0;
    std::vector<trecord> vRecords;

    ~CRNGCodex()
    {
		for (auto record : vRecords)
        {
            // data uses malloc so free all the memory in all of data.
            if(record.len > 0)
            {
                std::cout << "Free " << record.len << " hId " << record.hasherId << " locId "  << record.locId << std::endl;
                free(record.data);
                std::cout << "Free " << record.len << " hId " << record.hasherId << " locId "  << record.locId << std::endl;

            }
        }
	}

    void Write(fs::path path)
    {
        FILE *file = fsbridge::fopen(path, "wb");
        CAutoFile fileout(file, SER_DISK, CLIENT_VERSION);
        Serialize(fileout);
    }

    template <typename Stream>
    void Serialize(Stream& s) const
    {
        s << mapType;
        s << mapLoc;
        s << mapSrc;

        assert(vRecords.size() < UINT_MAX);
        unsigned int numRecords = (unsigned int) vRecords.size();
        s << (unsigned int) numRecords;

        for (unsigned int i = 0; i < numRecords; i++)
        {
            s << vRecords[i].hasherId;
            s << vRecords[i].hasherType;
            s << vRecords[i].locId;
            s << vRecords[i].srcId;
            s << vRecords[i].len;
            s.write(vRecords[i].data, vRecords[i].len);
        }
    }

    template <typename Stream>
    void Unserialize(Stream& s)
    {
        s >> mapType;
        s >> mapLoc;
        s >> mapSrc;

        unsigned int numRecords;
        s >> numRecords;

        for (unsigned int i = 0; i < numRecords; i++)
        {
            trecord record;
            s >> record.hasherId;
            s >> record.hasherType;
            s >> record.locId;
            s >> record.srcId;
            s >> record.len;

            record.data = (char *)malloc(sizeof(char) * record.len);
            s.read(record.data, record.len);
            vRecords.push_back(record);
        }
    }

    size_t AddRecord(unsigned int newHasherId, std::string newHasher, std::string newLoc, std::string newSrc, 
const unsigned char* newData, size_t len)
    {
        trecord record;
        record.hasherId = newHasherId;

        if (mapType.find(newHasher) == mapType.end())
        {
            mapType[newHasher] = ++typeCtr;
        }
        record.hasherType = mapType[newHasher];

        if (mapLoc.find(newLoc) == mapLoc.end())
        {
            mapLoc[newLoc] = ++locCtr;
        }
        record.locId = mapLoc[newLoc];

        if (mapSrc.find(newSrc) == mapSrc.end())
        {
            mapSrc[newSrc] = ++srcCtr;
        }
        record.srcId = mapSrc[newSrc];

        record.len = len;
        char* data = (char *)malloc(sizeof(char) * len);
        memcpy(data, newData, len);
        record.data = data;

        vRecords.push_back(record);

        return vRecords.size();
    }
};


class CRNGScribe
{

public:
    size_t MAX_MEM_SIZE = 100000;
    int64_t startTime = GetSystemTimeInSeconds();
    unsigned int currpart = 0;
    CRNGCodex* currCodex;


    CRNGScribe()
    {
        currCodex = new CRNGCodex();
    }


    Mutex mem_lock;
    void AddRecord(unsigned int hasherId, std::string name, std::string loc, std::string src, 
const unsigned char* data, size_t len)
    {
        currCodex->AddRecord(hasherId, name, loc, src, data, len);

        // std::cout << "write1" << std::endl; 

        if (mem_lock.try_lock())
        {
            if (currCodex->vRecords.size() >= MAX_MEM_SIZE)
            {
                CRNGCodex* codexToWrite = currCodex;
                currCodex = new CRNGCodex();
                // mem_lock.unlock();

                currpart += 1;
                std::string sdir = "/tmp/rngbook_"+std::to_string(this->startTime);

                if (!fs::exists(sdir))
                    fs::create_directories(sdir);
                
                std::string spath = sdir+"/"+std::to_string(currpart)+".dat";
                codexToWrite->Write(fs::path(spath));
                delete codexToWrite; // Since we have written the codex to disk we can free up its memory
                mem_lock.unlock();
            }
            else
            {
                mem_lock.unlock();
            }
        }
    }

};

#endif // BITCOIN_RNG_TRACER_H