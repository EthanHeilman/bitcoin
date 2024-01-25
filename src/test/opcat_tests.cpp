// Copyright (c) 2013-2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/tx_check.h>
#include <consensus/validation.h>
#include <hash.h>
#include <pubkey.h>
#include <script/interpreter.h>
#include <script/script_error.h>
#include <script/standard.h>
#include <serialize.h>
#include <streams.h>
#include <test/util/setup_common.h>
#include <test/util/transaction_utils.h>
#include <util/strencodings.h>
#include <util/system.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <univalue.h>
#include <version.h>

#include <boost/test/unit_test.hpp>


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

    CScript scriptPubkey = CScript() << OP_1 << ToByteVector(builder.GetOutput());

    TaprootSpendData spendData = builder.GetSpendData();
    CAmount nValue = 1;
    const CTransaction txCredit{BuildCreditingTransaction(scriptPubkey, nValue)};

    CScript scriptSig = CScript(); // Script sig is always size 0 and empty in tapscript
    CMutableTransaction txSpend = BuildSpendingTransaction(scriptSig, CScriptWitness(), txCredit);
    CScriptWitness& scriptWitness = txSpend.vin[0].scriptWitness;

    auto controlblocks = spendData.scripts[{witVerifyScript, TAPROOT_LEAF_TAPSCRIPT}];
    auto controlblock = *(controlblocks.begin());

    for (auto witDatum : witData) {
        scriptWitness.stack.push_back(witDatum);
    }
    scriptWitness.stack.push_back(witVerifyScript);
    scriptWitness.stack.push_back(controlblock);

    unsigned int flags = SCRIPT_VERIFY_P2SH;
    flags += SCRIPT_VERIFY_WITNESS;
    flags += SCRIPT_VERIFY_TAPROOT;
    flags += SCRIPT_VERIFY_TAPSCRIPT_OP_CAT;
    ScriptError err;

    bool success = VerifyScript(
        txSpend.vin[0].scriptSig,
        txCredit.vout[0].scriptPubKey,
        &txSpend.vin[0].scriptWitness,
        flags,
        MutableTransactionSignatureChecker(&txSpend, 0, txCredit.vout[0].nValue, MissingDataBehavior::ASSERT_FAIL),
        &err);

    BOOST_CHECK_MESSAGE(success == expect, message);
    BOOST_CHECK_MESSAGE(err == scriptError, "test failed: " + ScriptErrorString(err) + " expected: " + message);
}

BOOST_FIXTURE_TEST_SUITE(opcat_tests, BasicTestingSetup)

// Most of the OP_CAT tests live in data/script_tests.json

BOOST_AUTO_TEST_CASE(cat_simple)
{
    std::vector<std::vector<unsigned char>> witData;
    witData.push_back(ParseHex("aa"));
    witData.push_back(ParseHex("bbbb"));

    std::vector<unsigned char> witVerifyScript = { OP_CAT,
    OP_PUSHDATA1, 0x03, 0xaa, 0xbb, 0xbb,
    OP_EQUAL };

    DoTest(witVerifyScript, witData, SCRIPT_ERR_OK, "cat_simple");
}


BOOST_AUTO_TEST_CASE(cat_empty_stack)
{
    // Ensures that OP_CAT successfully handles concatenating two empty stack elements
    std::vector<std::vector<unsigned char>> witData;
    witData.push_back({});
    witData.push_back({});

    std::vector<unsigned char> witVerifyScript =
    { OP_CAT,
    OP_PUSHDATA1, 0x00,
    OP_EQUAL };

    DoTest(witVerifyScript, witData, SCRIPT_ERR_OK, "cat_empty_stack");
}

BOOST_AUTO_TEST_CASE(cat_dup_test)
{
    // CAT DUP exhaustion attacks using all element sizes from 1 to 522
    // with CAT DUP repetitions up to 10. Ensures the correct error is thrown
    // or not thrown, as appropriate.
    unsigned int maxElementSize = 522;
    unsigned int maxDupsToCheck = 10;

    std::vector<std::vector<unsigned char>> witData;
    witData.push_back({});

    for (unsigned int elementSize = 1; elementSize <= maxElementSize; elementSize++) {
        std::vector<unsigned char> witVerifyScript;
        // increase the size of stack element by one byte
        witData.at(0).push_back(0x1A);
        for (unsigned int dups = 1; dups <= maxDupsToCheck; dups++){
            witVerifyScript.push_back(OP_DUP);
            witVerifyScript.push_back(OP_CAT);
            int expectedErr = SCRIPT_ERR_OK;
            unsigned int catedStackElementSize = witData.at(0).size()*pow(2, dups);
            if (catedStackElementSize > MAX_SCRIPT_ELEMENT_SIZE)
                expectedErr = SCRIPT_ERR_INVALID_STACK_OPERATION;
            if (elementSize > MAX_SCRIPT_ELEMENT_SIZE)
                expectedErr = SCRIPT_ERR_PUSH_SIZE;
            DoTest(witVerifyScript, witData, expectedErr, "cat_dup_test");
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
