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

UniValue read_json(const std::string& jsondata);

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

void DoTest(std::vector<unsigned char> witVerifyScript, std::vector<std::vector<unsigned char>> witData)
{
    auto privkey = ParseHex("6b973d88838f27366ed61c9ad6367663045cb456e28335c109e30717ae0c6baa");
    CKey key;
    key.Set(privkey.begin(), privkey.end(), true);

    TaprootBuilder builder;
    int depth = 0;
    builder.Add(depth, witVerifyScript, TAPROOT_LEAF_TAPSCRIPT, /*track=*/true);
    builder.Finalize(XOnlyPubKey(key.GetPubKey()));
    assert(builder.HasScripts());
    assert(builder.IsComplete());

    // WitnessV1Taproot witOut = builder.GetOutput(); // std::cout << "witOut: " << HexStr(witOut)<< std::endl;
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

    std::cout << "err:" << err << std::endl;
    assert(success);
}

BOOST_FIXTURE_TEST_SUITE(opcat_tests, BasicTestingSetup)


BOOST_AUTO_TEST_CASE(cat_simple)
{
    std::vector<std::vector<unsigned char>> witData;
    witData.push_back({OP_1});
    witData.push_back({OP_1, OP_1, OP_1});
    witData.push_back(ParseHex("eeffeeff"));
    witData.push_back(ParseHex("aa"));
    witData.push_back(ParseHex("bb"));

    std::vector<unsigned char> witVerifyScript = {OP_CAT, OP_CAT, OP_DUP, OP_DROP, OP_PUSHDATA1, 0x06, 0xee, 0xff, 0xee, 0xff, 0xaa, 0xbb, OP_EQUAL, OP_NIP, OP_NIP};

    DoTest(witVerifyScript, witData);
}


BOOST_AUTO_TEST_CASE(cat_dup)
{
    std::vector<std::vector<unsigned char>> witData;
    witData.push_back({OP_1});
    witData.push_back(ParseHex("00"));

    std::vector<unsigned char> witVerifyScript = {OP_HASH256, OP_DUP, OP_SHA1, OP_CAT, OP_DUP, OP_CAT, OP_DUP, OP_CAT, OP_DROP};

    DoTest(witVerifyScript, witData);
}


BOOST_AUTO_TEST_CASE(cat_alt)
{
    std::vector<std::vector<unsigned char>> witData;
    witData.push_back({OP_1});
    witData.push_back(ParseHex("01"));
    witData.push_back(ParseHex("00"));

    std::vector<unsigned char> witVerifyScript = {OP_HASH256, OP_DUP, OP_SHA1, OP_CAT, OP_DUP, OP_CAT, OP_DUP, OP_CAT, OP_TOALTSTACK, OP_HASH256, OP_DUP, OP_CAT, OP_TOALTSTACK};

    DoTest(witVerifyScript, witData);
}


BOOST_AUTO_TEST_SUITE_END()
