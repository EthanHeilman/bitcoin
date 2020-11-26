#ifndef BITCOIN_RNG_ENTROPY_SOURCE_H
#define BITCOIN_RNG_ENTROPY_SOURCE_H

#include <string>

class CEntropySource
{
public:
    std::string name;
    const unsigned char* data;
    size_t len;

public:
    CEntropySource(const unsigned char* sourceData, size_t sourceLen, std::string sourceName)
    {
        name = sourceName;
        data = sourceData;
        len = sourceLen;
    };
};

class ESInt64 {
public:
  std::string name;
  int64_t data;

  ESInt64(int64_t newData, std::string newName)
  {
      name = newName;
      data = newData;
  }

  ESInt64(uint64_t newData, std::string newName)
  {
      name = newName;
      data = newData;
  }

};

#endif // BITCOIN_RNG_ENTROPY_SOURCE_H
