#include <rng/entropysource.h>
#include <string>


CEntropySource::CEntropySource(const unsigned char* sourceData, size_t sourceLen, std::string sourceName)
{
    name = sourceName;
    data = sourceData;
    len = sourceLen;
}

ESInt64::ESInt64(int64_t newData, std::string newName)
{
    name = newName;
    data = newData;
}

ESInt64::ESInt64(uint64_t newData, std::string newName)
{
    name = newName;
    data = newData;
}

ESInt32::ESInt32(uint32_t newData, std::string newName)
{
    name = newName;
    data = newData;
}