// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <test/fuzz/FuzzedDataProvider.h>
#include <test/fuzz/fuzz.h>
#include <test/fuzz/util.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

bool CastToBool(const std::vector<unsigned char>& vch);


void RunOnInput(std::vector<unsigned char> witVerifyScript, std::vector<std::vector<unsigned char>> witData)
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
}


FUZZ_TARGET(script_interpreter)
{
    FuzzedDataProvider fuzzed_data_provider(buffer.data(), buffer.size());
    {
        std::string input = fuzzed_data_provider.ConsumeRemainingBytesAsString();
        std::vector<unsigned char> witVerifyScript;
        std::vector<std::vector<unsigned char>> witData;

        getline(input, witVerifyScript, ' ');
        while ( input ) {
            std::vector<unsigned char> datum;
            getline(input, datum, ' ' );
            witData.push_back(datum);
        }

        RunOnInput(witVerifyScript, witData);
    }
    {
        (void)CastToBool(ConsumeRandomLengthByteVector(fuzzed_data_provider));
    }
}
