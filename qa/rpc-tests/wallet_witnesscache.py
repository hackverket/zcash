#!/usr/bin/env python2
# Copyright (c) 2016 The Zcash developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import *
from time import *

class WitnessCacheTest (BitcoinTestFramework):

    def setup_chain(self):
        print("Initializing test directory "+self.options.tmpdir)
        initialize_chain_clean(self.options.tmpdir, 4)

    # Start nodes with -regtestprotectcoinbase to set fCoinbaseMustBeProtected to true.
    def setup_network(self, split=False):
        self.nodes = start_nodes(3, self.options.tmpdir, extra_args=[['-regtestprotectcoinbase', '-debug=zrpc']] * 3 )
        connect_nodes_bi(self.nodes,0,1)
        connect_nodes_bi(self.nodes,1,2)
        connect_nodes_bi(self.nodes,0,2)
        self.is_network_split=False
        self.sync_all()

    def wait_and_assert_operationid_status(self, mynode, myopid, in_status='success', in_errormsg=None):
        print('waiting for async operation {}'.format(myopid))
        opids = []
        opids.append(myopid)
        timeout = 120
        status = None
        errormsg = None
        txid = None
        for x in xrange(1, timeout):
            results = mynode.z_getoperationresult(opids)
            if len(results)==0:
                sleep(1)
            else:
                status = results[0]["status"]
                if status == "success":
                    txid = results[0]['result']['txid']
                if status == "failed":
                    errormsg = results[0]['error']['message']
                break
        print('...returned status: {}'.format(status))
        assert_equal(in_status, status)
        if errormsg is not None:
            assert(in_errormsg is not None)
            assert_equal(in_errormsg in errormsg, True)
            print('...returned error: {}'.format(errormsg))
        return txid

    def run_test (self):
        print "Mining blocks..."
        self.nodes[0].generate(4)

        walletinfo = self.nodes[0].getwalletinfo()
        assert_equal(walletinfo['immature_balance'], 40)
        assert_equal(walletinfo['balance'], 0)

        self.sync_all()
        self.nodes[1].generate(102)
        self.sync_all()

        assert_equal(self.nodes[0].getbalance(), 40)
        assert_equal(self.nodes[1].getbalance(), 20)
        assert_equal(self.nodes[2].getbalance(), 0)

        # Node 0 creates a joinsplit transaction
        mytaddr0 = self.nodes[0].getnewaddress()
        myzaddr0 = self.nodes[0].z_getnewaddress()
        recipients = []
        recipients.append({"address":myzaddr0, "amount": Decimal('10.0') - Decimal('0.0001')})
        myopid = self.nodes[0].z_sendmany(mytaddr0, recipients)
        self.wait_and_assert_operationid_status(self.nodes[0], myopid)

        # Sync up mempools.
        self.sync_all()
        self.nodes[0].generate(1)
        self.sync_all()

        # Stop nodes.
        stop_nodes(self.nodes)
        wait_bitcoinds()

        # Relaunch nodes and partition network into two:
        # A: node 0
        # B: node 1, 2
        self.nodes = start_nodes(3, self.options.tmpdir, extra_args=[['-regtestprotectcoinbase', '-debug=zrpc']] * 3 )
        connect_nodes_bi(self.nodes,1,2)

        # Partition A, node 0 mines an empty block
        self.nodes[0].generate(1)

        # Partition B, node 1 mines two blocks
        self.nodes[1].generate(1)
        mytaddr1 = self.nodes[1].getnewaddress()
        #myzaddr1 = self.nodes[1].z_getnewaddress()
        recipients = []
        recipients.append({"address":myzaddr0, "amount": Decimal('10.0') - Decimal('0.0001')})
        myopid = self.nodes[1].z_sendmany(mytaddr1, recipients)
        txid = self.wait_and_assert_operationid_status(self.nodes[1], myopid)
        self.nodes[1].generate(1)

        # Check that Partition B is one block ahead and that they have different tips
        assert_equal(self.nodes[0].getblockcount() + 1, self.nodes[1].getblockcount())
        assert( self.nodes[0].getbestblockhash() != self.nodes[1].getbestblockhash())

        # Shut down all nodes so any in-memory state is saved to disk
        stop_nodes(self.nodes)
        wait_bitcoinds()

        # Relaunch nodes and reconnect the entire network
        self.nodes = start_nodes(3, self.options.tmpdir, extra_args=[['-regtestprotectcoinbase', '-debug=zrpc']] * 3 )
        connect_nodes_bi(self.nodes,0, 1)
        connect_nodes_bi(self.nodes,1, 2)
        connect_nodes_bi(self.nodes,0, 2)
        self.nodes[1].generate(10)
        self.sync_all()
        stop_nodes(self.nodes)
        wait_bitcoinds()

        self.nodes = start_nodes(3, self.options.tmpdir, extra_args=[['-reindex', '-regtestprotectcoinbase', '-debug=zrpc']] * 3 )
        sleep(5)
        # If reindex fails, sync_all() will throw an exception.
        self.sync_all()

if __name__ == '__main__':
    WitnessCacheTest().main()
