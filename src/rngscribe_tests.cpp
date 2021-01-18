// // Copyright (c) 2012-2020 The Bitcoin Core developers
// // Distributed under the MIT software license, see the accompanying
// // file COPYING or http://www.opensource.org/licenses/mit-license.php.
// #include <rng/rngscribe.h>
// #include <test/util/setup_common.h>
// #include <util/string.h>
// #include <hash.h>
// #include <random.h>
// #include <streams.h>



// #include <boost/test/unit_test.hpp>

// #include <string>



// // test/test_bitcoin --log_level=all --run_test=rngscribe_tests -- DEBUG_LOG_OUT

// BOOST_FIXTURE_TEST_SUITE(rngscribe_tests, BasicTestingSetup)

// BOOST_AUTO_TEST_CASE(simple_serialization)
// {
//     CRNGCodex outRecorder;
//     CRNGCodex inRecorder;

//     CDataStream stream(1,1);

//     BOOST_CHECK_EQUAL((int)outRecorder.vRecords.size(), 0);

//     const unsigned char* test_data1 = (const unsigned char*) NULL;
//     size_t data_len1 = 0;
//     outRecorder.AddRecord(1, "test_hasher1", "test_locationA", "test_sourceA", test_data1, data_len1);

//     const unsigned char* test_data2 = (const unsigned char*) "12345";
//     size_t data_len2 = 5;
//     outRecorder.AddRecord(2, "test_hasher2", "test_locationB", "test_sourceA", test_data2, data_len2);

//     const unsigned char* test_data3 = (const unsigned char*) "testdata testdata testdata testdata ";
//     size_t data_len3 = 4*9;
//     outRecorder.AddRecord(1, "test_hasher1", "test_locationC", "test_sourceB", test_data3, data_len3);

//     BOOST_CHECK_EQUAL((int)outRecorder.vRecords.size(), 3);
//     outRecorder.Serialize(stream);
    
//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords.size(), 0);
//     BOOST_CHECK_EQUAL((int)inRecorder.mapType.size(), 0);

//     inRecorder.Unserialize(stream);

//     // Check map hasher
//     BOOST_CHECK_EQUAL((int)inRecorder.mapType.size(), 2);
//     BOOST_CHECK_EQUAL((int)inRecorder.mapType["test_hasher1"], 1);
//     BOOST_CHECK_EQUAL((int)inRecorder.mapType["test_hasher2"], 2);

//     BOOST_CHECK_EQUAL((int)inRecorder.mapLoc.size(), 3);
//     BOOST_CHECK_EQUAL((int)inRecorder.mapLoc["test_locationA"], 1);
//     BOOST_CHECK_EQUAL((int)inRecorder.mapLoc["test_locationB"], 2);
//     BOOST_CHECK_EQUAL((int)inRecorder.mapLoc["test_locationC"], 3);

//     BOOST_CHECK_EQUAL((int)inRecorder.mapSrc.size(), 2);
//     BOOST_CHECK_EQUAL((int)inRecorder.mapSrc["test_sourceA"], 1);
//     BOOST_CHECK_EQUAL((int)inRecorder.mapSrc["test_sourceB"], 2);

//     // Check vRecords
//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords.size(), 3);

//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[0].hasherId, 1);
//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[0].hasherType, 1);

//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[0].locId, 1);
//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[0].srcId, 1);
//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[0].len, 0);

//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[1].hasherId, 2);
//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[1].hasherType, 2);

//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[1].locId, 2);
//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[1].srcId, 1);
//     BOOST_CHECK_EQUAL(inRecorder.vRecords[1].len, data_len2);
//     BOOST_CHECK_EQUAL(inRecorder.vRecords[1].data, "12345");

//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[2].hasherId, 1);
//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[2].hasherType, 1);
//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[2].locId, 3);
//     BOOST_CHECK_EQUAL((int)inRecorder.vRecords[2].srcId, 2);
//     BOOST_CHECK_EQUAL(inRecorder.vRecords[2].len, data_len3);
//     BOOST_CHECK_EQUAL(inRecorder.vRecords[2].data, "testdata testdata testdata testdata ");
// }



// BOOST_AUTO_TEST_SUITE_END()
