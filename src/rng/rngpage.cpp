// Copyright (c) 2014-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <rng/rngpage.h>
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



CRNGPage::~CRNGPage()
{
    for (auto record : vRecords)
    {
        // data uses malloc so free all the memory in all of data.
        if(record.len > 0)
        {
            // std::cout << "Free " << record.len << " hId " << record.hasherId << " locId "  << record.locId << std::endl;
            free(record.data);
            // std::cout << "Free " << record.len << " hId " << record.hasherId << " locId "  << record.locId << std::endl;
        }
    }
}

void CRNGPage::Write(fs::path path)
{
    FILE *file = fsbridge::fopen(path, "wb");
    CAutoFile fileout(file, SER_DISK, CLIENT_VERSION);
    Serialize(fileout);
}

template <typename Stream>
void CRNGPage::Serialize(Stream& s) const
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
void CRNGPage::Unserialize(Stream& s)
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

size_t CRNGPage::AddRecord(unsigned int newHasherId, std::string newHasher, std::string newLoc, std::string newSrc, 
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