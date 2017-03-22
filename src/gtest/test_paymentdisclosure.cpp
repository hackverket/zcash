#include <gtest/gtest.h>

#include "main.h"
#include "utilmoneystr.h"
#include "chainparams.h"
#include "utilstrencodings.h"
#include "zcash/Address.hpp"
#include "wallet/wallet.h"
#include "amount.h"
#include <memory>
#include <string>
#include <set>
#include <vector>
#include <boost/filesystem.hpp>
#include "util.h"

#include "paymentdisclosure.h"
#include "paymentdisclosuredb.h"

#include "sodium.h"
// To run tests:
// ./zcash-gtest --gtest_filter="paymentdisclosure.*"


static uint256 random_uint256()
{
    uint256 ret;
    randombytes_buf(ret.begin(), 32);

    return ret;
}


TEST(paymentdisclosure, testnet) {
    ECC_Start();
    SelectParams(CBaseChainParams::TESTNET);
    
    boost::filesystem::path pathTemp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    boost::filesystem::create_directories(pathTemp);
    mapArgs["-datadir"] = pathTemp.string();

    std::cout << "Test payment disclosure database in folder: " << pathTemp.native() << std::endl;
    
    PaymentDisclosureDB mydb(pathTemp);
 
    //std::map<PaymentDisclosureKey, PaymentDisclosureInfo> m;

    for (int i=0; i<100; i++) {
        size_t js = random_uint256().GetCheapHash() % std::numeric_limits<size_t>::max();
        uint8_t n = random_uint256().GetCheapHash() % std::numeric_limits<uint8_t>::max();
        PaymentDisclosureKey key { random_uint256(), js, n};
        PaymentDisclosureInfo info;
        info.esk = random_uint256();
        info.joinSplitPrivKey = random_uint256();
        bool b = mydb.Put(key, info);
        ASSERT_TRUE(b);

        PaymentDisclosureInfo info2;
        b = mydb.Get(key, info2);
        ASSERT_TRUE(b);
        ASSERT_EQ(info, info2);

        info2.esk = random_uint256();
        info2.joinSplitPrivKey = random_uint256();
        ASSERT_NE(info, info2);

    }

#if 1
    mydb.DebugDumpAllStdout();
#endif
}
