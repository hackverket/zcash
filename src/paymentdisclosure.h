// Copyright (c) 2017 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ZCASH_PAYMENTDISCLOSURE_H
#define ZCASH_PAYMENTDISCLOSURE_H

//#include "consensus/params.h"

#include "uint256.h"
#include "clientversion.h"
#include "serialize.h"
#include "streams.h"
#include "version.h"

// JSOutPoint
#include "wallet/wallet.h"

#include <cstdint>
#include <string>


#define PAYMENT_DISCLOSURE_VERSION_EXPERIMENT 0

struct PaymentDisclosurePayload {
    uint8_t version;        // 0 = experimental, 1 = first production version, etc.
    uint256 esk;            // zcash/NoteEncryption.cpp
    uint256 txid;           // primitives/transaction.h
    size_t js;              // Index into CTransaction.vjoinsplit
    uint8_t n;              // Index into JSDescription fields of length ZC_NUM_JS_OUTPUTS
    std::string message;     // parameter to RPC call

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(version);
        READWRITE(esk);
        READWRITE(txid);
        READWRITE(js);
        READWRITE(n);
        READWRITE(message);
    }
};

struct PaymentDisclosure {
    PaymentDisclosurePayload            payload;
    boost::array<unsigned char, 64>     payloadSig;
    
    // serialize doesn't like char buffer.
    //unsigned char               payloadSig[64];

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(payload);
        READWRITE(payloadSig);
    }
};


// class PaymentDisclosureKey : JSOutPoint {
// public:
//     std::string ToString() const;
// };
typedef JSOutPoint PaymentDisclosureKey;
// TODO: Use JSOutPoint for now to avoid duplication of effort...
// struct PaymentDisclosureKey {
//     uint256 txid            // primitives/transaction.h
//     size_t js;              // Index into CTransaction.vjoinsplit
//     uint8_t n;              // Index into JSDescription fields of length ZC_NUM_JS_OUTPUTS
// };

struct PaymentDisclosureInfo {
    uint8_t version;          // 0 = experimental, 1 = first production version, etc.
    uint256 esk;              // zcash/NoteEncryption.cpp
    uint256 joinSplitPrivKey; // primitives/transaction.h

    PaymentDisclosureInfo() : version(PAYMENT_DISCLOSURE_VERSION_EXPERIMENT) {
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(version);
        READWRITE(esk);
        READWRITE(joinSplitPrivKey);
    }

    std::string ToString() const;
    // TODO: ToHexString() method
    friend bool operator==(const PaymentDisclosureInfo& a, const PaymentDisclosureInfo& b) {
        return (a.version == b.version && a.esk == b.esk && a.joinSplitPrivKey == b.joinSplitPrivKey);
    }

    friend bool operator!=(const PaymentDisclosureInfo& a, const PaymentDisclosureInfo& b) {
        return !(a == b);
    }

};


#endif // ZCASH_PAYMENTDISCLOSURE_H
