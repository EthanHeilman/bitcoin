// Copyright (c) 2013-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/tx_check.h>
#include <consensus/validation.h>
#include <hash.h>
#include <pubkey.h>
#include <script/interpreter.h>
#include <script/standard.h>
#include <serialize.h>
#include <streams.h>
#include <test/data/ctvhash.json.h>
#include <test/util/setup_common.h>
#include <test/util/transaction_utils.h>
#include <util/strencodings.h>
#include <util/system.h>

#include <version.h>

#include <iostream>

#include <boost/test/unit_test.hpp>

#include <univalue.h>

#include <fstream>

#include <cmath>


void PrintControlblock(auto controlblock)
{
    std::cout << "controlblock: " << controlblock.size() << std::endl;
    for (auto ch : controlblock) {
        printf("%02x", ch);
    }
    std::cout << std::endl;
}

void PrintSpendData(TaprootSpendData spendData)
{
    std::cout << "spendData.internal_key: " << HexStr(spendData.internal_key) << std::endl;
    std::cout << "spendData.scripts.size(): " << spendData.scripts.size() << std::endl;
    for (const auto& [leaf_script, control_block] : spendData.scripts) {
        std::cout << "leaf_script.first.size(): " << leaf_script.first.size() << " leaf_script int (2): " << leaf_script.second << std::endl;
        for (auto lsfirst : leaf_script.first) {
            printf("%02x", lsfirst);
        }
        std::cout << std::endl;

        std::cout << "control_block.size(): " << control_block.size() << std::endl;
        for (auto cbs : control_block) {
            std::cout << "cbs.size(): " << cbs.size() << std::endl;
            for (auto ch : cbs) {
                printf("%02x", ch);
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}


void WriteWitInput(std::string fname, std::vector<unsigned char> witVerifyScript, std::vector<std::vector<unsigned char>> witData){
    std::ofstream f;
    f.open("src/test/fuzz_files/"+fname);
    f << std::string(witVerifyScript.begin(), witVerifyScript.end());
    for (auto datum : witData){
        f << " " <<  std::string(datum.begin(), datum.end());
    }
    f.close();
}


void PrintJSON(std::vector<std::vector<unsigned char>> scriptWitness, WitnessV1Taproot witOut){

    for (auto line : scriptWitness){
        std::cout << " " <<  HexStr(line) << std::endl;
    }
    std::cout << " " <<  HexStr(witOut) << std::endl;
}

void DoTest(std::vector<unsigned char> witVerifyScript, std::vector<std::vector<unsigned char>> witData, int scriptError, const std::string& message)
{
    bool expect = (scriptError == SCRIPT_ERR_OK);

    auto privkey = ParseHex("6b973d88838f27366ed61c9ad6367663045cb456e28335c109e30717ae0c6baa");
    CKey key;
    key.Set(privkey.begin(), privkey.end(), true);

    TaprootBuilder builder;
    int depth = 0;
    builder.Add(depth, witVerifyScript, TAPROOT_LEAF_TAPSCRIPT, /*track=*/true);
    builder.Finalize(XOnlyPubKey(key.GetPubKey()));
    assert(builder.HasScripts());
    assert(builder.IsComplete());

    // WitnessV1Taproot witOut = builder.GetOutput();
    // std::cout << "witOut: " << HexStr(witOut)<< std::endl;
    CScript scriptPubkey = CScript() << OP_1 << ToByteVector(builder.GetOutput());

    TaprootSpendData spendData = builder.GetSpendData();
    PrintSpendData(spendData);

    CAmount nValue = 1;
    const CTransaction txCredit{BuildCreditingTransaction(scriptPubkey, nValue)};

    CScript scriptSig = CScript(); // Script sig is only size 0 and empty in tapscript
    CMutableTransaction txSpend = BuildSpendingTransaction(scriptSig, CScriptWitness(), txCredit);
    CScriptWitness& scriptWitness = txSpend.vin[0].scriptWitness;

    auto controlblocks = spendData.scripts[{witVerifyScript, TAPROOT_LEAF_TAPSCRIPT}];
    auto controlblock = *(controlblocks.begin());
    PrintControlblock(controlblock);

    for (auto witDatum : witData) {
        scriptWitness.stack.push_back(witDatum);
    }
    scriptWitness.stack.push_back(witVerifyScript);
    scriptWitness.stack.push_back(controlblock);

    PrintJSON(scriptWitness.stack, builder.GetOutput());

    // unsigned int flags = 0;
    unsigned int flags = SCRIPT_VERIFY_WITNESS;
    flags += SCRIPT_VERIFY_TAPROOT;
    flags += SCRIPT_VERIFY_P2SH;
    ScriptError err;

    bool success = VerifyScript(
        txSpend.vin[0].scriptSig,
        txCredit.vout[0].scriptPubKey,
        &txSpend.vin[0].scriptWitness,
        flags,
        MutableTransactionSignatureChecker(&txSpend, 0, txCredit.vout[0].nValue, MissingDataBehavior::ASSERT_FAIL),
        &err);

    BOOST_CHECK_MESSAGE(success == expect, message);
    BOOST_CHECK_MESSAGE(err == scriptError, std::to_string(err) + " where " + std::to_string(scriptError) + " expected: " + message);
    assert(err == scriptError); // ERH - remove this later
    // std::cout << "err:" << err << std::endl;
}

BOOST_FIXTURE_TEST_SUITE(opcat_tests, BasicTestingSetup)



BOOST_AUTO_TEST_CASE(cat_simple)
{
    std::vector<std::vector<unsigned char>> witData;
    witData.push_back(ParseHex("aa"));
    witData.push_back(ParseHex("bb"));

    std::vector<unsigned char> witVerifyScript = 
    { OP_CAT, 
    OP_PUSHDATA1, 0x02, 0xaa, 0xbb,
    OP_EQUAL };

    DoTest(witVerifyScript, witData, SCRIPT_ERR_OK, "cat_simple");
}

BOOST_AUTO_TEST_CASE(cat_twice)
{
    std::vector<std::vector<unsigned char>> witData;
    witData.push_back(ParseHex("eeffeeff"));
    witData.push_back(ParseHex("aa"));
    witData.push_back(ParseHex("bbcc"));

    std::vector<unsigned char> witVerifyScript = 
    { OP_CAT, OP_CAT, OP_DUP, OP_DROP, 
    OP_PUSHDATA1, 0x06, 0xee, 0xff, 0xee, 0xff, 0xaa, 0xbb, 0xcc, 
    OP_EQUAL };

    DoTest(witVerifyScript, witData, SCRIPT_ERR_OK, "cat_twice");
}


BOOST_AUTO_TEST_CASE(cat_many)
{
    std::vector<std::vector<unsigned char>> witData;
    witData.push_back(ParseHex("aa"));
    witData.push_back(ParseHex("bb"));
    witData.push_back(ParseHex("cc"));
    witData.push_back(ParseHex("dd"));
    witData.push_back(ParseHex("ee"));
    witData.push_back(ParseHex("ff"));

    std::vector<unsigned char> witVerifyScript = 
    { OP_CAT, OP_CAT, OP_CAT, OP_CAT, OP_CAT,
    OP_PUSHDATA1, 0x06, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    OP_EQUAL };

    DoTest(witVerifyScript, witData, SCRIPT_ERR_OK, "cat_many");
}


BOOST_AUTO_TEST_CASE(cat_dup)
{
    int maxElementSize = 0;
    int maxDupsToCheck = 0;

    std::vector<std::vector<unsigned char>> witData;
    witData.push_back({});

    for (int elementSize = 1; elementSize <= maxElementSize; elementSize++) {
        witData.at(0).push_back(0x1A); // increase the size of element by one byte

        std::vector<unsigned char> witVerifyScript;
        for (int dups = 1; dups <= maxDupsToCheck; dups++){
            witVerifyScript.push_back(OP_DUP);
            witVerifyScript.push_back(OP_CAT);

            int expectedErr = SCRIPT_ERR_OK;
            
            if ((witData.at(0).size()*pow(2, dups)) > MAX_SCRIPT_ELEMENT_SIZE) {
                expectedErr = SCRIPT_ERR_INVALID_STACK_OPERATION;
            }
            if (elementSize > MAX_SCRIPT_ELEMENT_SIZE) {
                expectedErr = SCRIPT_ERR_PUSH_SIZE;
            }
            std::cout << "DUP CHECK " << witData.size() << " " << dups << " " << (witData.at(0).size()*pow(2, dups)) << " " << expectedErr << std::endl;
                
            DoTest(witVerifyScript, witData, expectedErr, "cat_dup");
        }
    }


    // PrintJSON("cat_dup.bin", witVerifyScript, witData);
    // DoTest(witVerifyScript, witData, SCRIPT_ERR_OK, "cat_dup");
}

BOOST_AUTO_TEST_CASE(cat_dup_big)
{
    std::vector<std::vector<unsigned char>> witData;
    witData.push_back(ParseHex("555"));

    std::vector<unsigned char> witVerifyScript = {OP_HASH256, OP_DUP, OP_CAT, OP_DUP, OP_CAT, OP_DUP, OP_CAT, OP_DUP, OP_CAT, OP_DUP, OP_CAT};

    DoTest(witVerifyScript, witData, SCRIPT_ERR_INVALID_STACK_OPERATION, "cat_dup_big");
}


BOOST_AUTO_TEST_CASE(cat_alt)
{
    std::vector<std::vector<unsigned char>> witData;

    witData.push_back(ParseHex("1FFE1234567890"));
    witData.push_back(ParseHex("00"));

    std::vector<unsigned char> witVerifyScript = {OP_HASH256, OP_DUP, OP_SHA1, OP_CAT, OP_DUP, OP_CAT, OP_TOALTSTACK, OP_HASH256, OP_DUP, OP_CAT, OP_TOALTSTACK, OP_FROMALTSTACK};

    DoTest(witVerifyScript, witData, SCRIPT_ERR_OK, "cat_alt");
}


BOOST_AUTO_TEST_CASE(script_fuzz)
{
    std::ifstream f;
    f.open("src/test/script_fuzz.bin");
    // std::ifstream f ("script_fuzz.bin");
    std::string str;
    f >> str;
    if ( f.is_open() ) {
        while ( f ) { // equivalent to myfile.good()
            std::getline (f, str);
            std::cout << str << '\n';
        }
    } else {
        std::cout << "Couldn't open file\n";
    }

    std::cout << "FILE: " << str << std::endl;
}


BOOST_AUTO_TEST_SUITE_END()
