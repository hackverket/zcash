// Copyright (c) 2017 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "rpcserver.h"
#include "init.h"
#include "main.h"
#include "script/script.h"
#include "script/standard.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"
#include "wallet.h"

#include <fstream>
#include <stdint.h>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <univalue.h>

#include "paymentdisclosure.h"

using namespace std;

void EnsureWalletIsUnlocked();
bool EnsureWalletIsAvailable(bool avoidException);

UniValue z_getpaymentdisclosure(const UniValue& params, bool fHelp)
{
    if (!EnsureWalletIsAvailable(fHelp))
        return NullUniValue;

    if (fHelp || params.size() != 1)
        throw runtime_error(
            "z_getpaymentdisclosure \"txid\" \"js_index\" \"output_index\" (\"message\") \n"
            "\nGenerate a payment disclosure for a given joinsplit output.\n"
            "\nEXPERIMENTAL FEATURE (TODO: ADD FLAG)\n"            
            "\nArguments:\n"
            "1. \"txid\"    (string, required) \n"
            "2. \"js_index\"    (string, required) \n"
            "3. \"output_index\"    (string, required) \n"
            "4. \"message\"    (string, optional) \n"
            "\nResult:\n"
            "\"path\"           (string) The full path of the destination file\n"
            "\nExamples:\n"
            + HelpExampleCli("dumpwallet", "\"test\"")
            + HelpExampleRpc("dumpwallet", "\"test\"")
        );

    LOCK2(cs_main, pwalletMain->cs_wallet);

    EnsureWalletIsUnlocked();

    return NullUniValue;
}

// TODO: This method does not rely on a wallet so could be somewhere else, e.g. rpcblockcain ?
UniValue z_validatepaymentdisclosure(const UniValue& params, bool fHelp)
{
    if (!EnsureWalletIsAvailable(fHelp))
        return NullUniValue;

    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "z_validatepaymentdisclosure \"paymentdisclosure\"\n"
            "\nValidates a payment disclosure.\n"
            "\nEXPERIMENTAL FEATURE (TODO: ADD FLAG)\n"            
            "\nArguments:\n"
            "1. \"paymentdisclosure\"     (string, required) Hex string\n"
            "\nExamples:\n"
            "\nValidate a payment diclosure\n"
            "\nAs a JSON-RPC call\n"
            + HelpExampleRpc("z_validatepaymentdisclosure", "\"hexblob\"")
        );

    LOCK2(cs_main, pwalletMain->cs_wallet);

    EnsureWalletIsUnlocked();

    return NullUniValue;
}
