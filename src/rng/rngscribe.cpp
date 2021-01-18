#include <rng/rngscribe.h>

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
#include <util/system.h>

#include <assert.h>
#include <vector>

#include <sync.h>

#include <fs.h>
#include <serialize.h>
#include <streams.h>

#include <clientversion.h>


CRNGScribe::CRNGScribe()
{
    currPage = new CRNGPage();
}



void CRNGScribe::AddRecord(unsigned int hasherId, std::string type, std::string loc, std::string src, 
const unsigned char* data, size_t len)
{
    // Don't record Static Env data since they may contain sensetive details
    if (loc.compare("RandAddStaticEnv") == 0) len=0;
    if (loc.compare("AddSockaddr") == 0) len=0;
    if (loc.compare("AddPath") == 0) len=0;
    if (loc.compare("AddFile") == 0) len=0;

    mem_lock.lock();
    currPage->AddRecord(hasherId, type, loc, src, data, len);
    mem_lock.unlock();
    if (mem_lock.try_lock())
    {
        if (currPage->vRecords.size() >= MAX_MEM_SIZE)
        {
            std::cout<<"WritePage called"<<std::endl;
            
            CRNGPage* codexToWrite = currPage;
            currPage = new CRNGPage();
            mem_lock.unlock();

            currpart += 1;
            // std::string sdir = "/tmp/rngbook_"+std::to_string(this->startTime);
            fs::path fdir =  GetDataDir() / fs::path("rngbook_"+std::to_string(this->startTime));

            if (!fs::exists(fdir))
                fs::create_directories(fdir);
            
            fs::path fpath = fdir / fs::path(""+std::to_string(currpart)+".dat");
            // std::string spath = sdir+"/"+std::to_string(currpart)+".dat";
            // codexToWrite->Write(fs::path(spath));
            codexToWrite->Write(fpath);
            delete codexToWrite; // Since we have written the codex to disk we can free up its memory
        }
        else
        {
            mem_lock.unlock();
        }
    }
}

void CRNGScribe::WritePage()
{
    mem_lock.lock();
    std::cout<<"WritePage called"<<std::endl;
    
    CRNGPage* codexToWrite = currPage;
    currPage = new CRNGPage();
    currpart += 1;
    // std::string sdir = "/tmp/rngbook_"+std::to_string(this->startTime);
    fs::path fdir =  GetDataDir() / fs::path("rngbook_"+std::to_string(this->startTime));


    if (!fs::exists(fdir))
        fs::create_directories(fdir);
    
    fs::path fpath = fdir / fs::path(""+std::to_string(currpart)+".dat");
    // std::string spath = sdir+"/"+std::to_string(currpart)+".dat";

    // codexToWrite->Write(fs::path(spath));
    codexToWrite->Write(fpath);
    delete codexToWrite; // Since we have written the codex to disk we can free up its memory
    mem_lock.unlock();
}

CRNGScribe& CRNGScribe::GetCRNGScribe() noexcept
{
    // This C++11 idiom relies on the guarantee that static variable are initialized
    // on first call, even when multiple parallel calls are permitted.
    static std::vector<CRNGScribe, secure_allocator<CRNGScribe>> g_rngscribe(1);
    return g_rngscribe[0];
}

