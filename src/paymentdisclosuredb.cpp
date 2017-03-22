// Copyright (c) 2017 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "paymentdisclosuredb.h"

//#include "main.h"
#include "util.h"

#include "leveldbwrapper.h"

//#include <stdint.h>
#include <boost/filesystem.hpp>

#include <iostream>

using namespace std;

static boost::filesystem::path emptyPath;

// C++11 delegated constructor
PaymentDisclosureDB::PaymentDisclosureDB() : PaymentDisclosureDB(emptyPath) {
}

PaymentDisclosureDB::PaymentDisclosureDB(const boost::filesystem::path& dbPath) {
    boost::filesystem::path path(dbPath);
     
     std::cout << "PaymentDisclosure: dbPath: " << dbPath.string() << std::endl;
     std::cout << "PaymentDisclosure: path:" << path.string()  << std::endl;

    if (path.empty()) {
        path /= GetDataDir() / "paymentdisclosure";
        std::cout << "PaymentDisclosure: is empty, so use default path:" << path.string()  << std::endl;
        //LogPrintf("PaymentDisclosure: using default path for database: %s\n", path.string());
    }

    TryCreateDirectory(path);
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, path.string(), &db);
    HandleError(status);
    LogPrintf("PaymentDisclosure: Opened LevelDB successfully\n");
}

PaymentDisclosureDB::~PaymentDisclosureDB() {
    if (db != nullptr) {
        delete db;
    }
}

bool PaymentDisclosureDB::Put(const PaymentDisclosureKey& key, const PaymentDisclosureInfo& info)
{
    // TODO: fail gracefully
    assert(db != nullptr);

    // TODO: check leveldb docs, maybe we don't need this?
    std::lock_guard<std::mutex> guard(lock_);

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(ssValue.GetSerializeSize(info));
    ssValue << info;
    leveldb::Slice slice(&ssValue[0], ssValue.size());

    leveldb::Status status = db->Put(writeOptions, key.ToString(), slice);
    HandleError(status);
    return true;
}

bool PaymentDisclosureDB::Get(const PaymentDisclosureKey& key, PaymentDisclosureInfo& info)
{
    // TODO: fail gracefully
    assert(db != nullptr);

    // TODO: check leveldb docs,
    // std::lock_guard<std::mutex> guard(lock_);

    std::string strValue;
    leveldb::Status status = db->Get(readOptions, key.ToString(), &strValue);
    if (!status.ok()) {
        if (status.IsNotFound())
            return false;
        LogPrintf("PaymentDisclosure: LevelDB read failure: %s\n", status.ToString());
        HandleError(status);
    }

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> info;
    } catch (const std::exception&) {
        return false;
    }
    return true;
}



// TODO: Remove debug methods
void PaymentDisclosureDB::DebugDumpAllStdout() {
    // TODO: fail gracefully
    assert(db != nullptr);

    // TODO: check leveldb docs, maybe we don't need this?
    std::lock_guard<std::mutex> guard(lock_);

    // Iterate over each item in the database and print them
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        cout << it->key().ToString() << " : " ; //<< it->value().ToString() << endl;

        try {
            std::string strValue = it->value().ToString();
            PaymentDisclosureInfo info;
            CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
            ssValue >> info;
            cout << info.ToString() << std::endl;
        } catch (const std::exception& e) {
            cout << e.what() << std::endl;
        }

    }
    
    if (false == it->status().ok())
    {
        cerr << "An error was found during the scan" << endl;
        cerr << it->status().ToString() << endl; 
    }
    
    delete it;
}

