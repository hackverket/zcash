// Copyright (c) 2017 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "paymentdisclosure.h"
#include "util.h"

// std::string PaymentDisclosureKey::ToString() const {
//     return strprintf("PaymentDisclosureKey(%s, %d, %d)", hash.ToString().substr(0,10), js, n);
// }

std::string PaymentDisclosureInfo::ToString() const {
    return strprintf("PaymentDisclosureInfo(version=%d, esk=%s, joinSplitPrivKey=%s)",
        version, esk.ToString(), joinSplitPrivKey.ToString());
}
