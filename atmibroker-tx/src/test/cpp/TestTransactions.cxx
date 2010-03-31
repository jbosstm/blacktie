 /*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
 * by the @authors tag. All rights reserved.
 * See the copyright.txt in the distribution for a
 * full listing of individual contributors.
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License, v. 2.1.
 * This program is distributed in the hope that it will be useful, but WITHOUT A
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <string>
#include <sstream>

#include "TestAssert.h"
#include "TestTransactions.h"
#include "txi.h"
#include "tx.h"
#include "testrm.h"
#include "ThreadLocalStorage.h"
#include "userlogc.h"
#include "testTxAvoid.h"

#ifndef WIN32
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"
#endif

//check_info(const char *msg, int rv, COMMIT_RETURN cr, TRANSACTION_CONTROL tc, TRANSACTION_TIMEOUT tt, TRANSACTION_STATE ts)
#define CHECKINFO(msg, rv, cr, tc, tt, ts)	{\
	TXINFO txi;	\
	BT_ASSERT_MESSAGE(msg, rv == tx_info(&txi));	\
	userlogc_debug("TestTransactions::check_info begin %ld=%ld %ld=%ld %ld=%ld %ld=%ld",	\
		txi.when_return, (long) (cr),	\
		txi.transaction_control, (long) (tc),	\
		txi.transaction_timeout, (long) (tt),	\
		txi.transaction_state, (long) (ts));	\
	if ((long) (cr) >= 0l) BT_ASSERT_MESSAGE(msg, txi.when_return == (long) (cr));	\
	if ((long) (tc) >= 0l) BT_ASSERT_MESSAGE(msg, txi.transaction_control == (long) (tc));	\
    if ((long) (tt) >= 0l) BT_ASSERT_MESSAGE(msg, txi.transaction_timeout == (long) (tt));	\
    if ((long) (ts) >= 0l) BT_ASSERT_MESSAGE(msg, txi.transaction_state == (long) (ts));}


void TestTransactions::setUp()
{
	init_ace();

	txx_stop();
	initEnv();

	TestFixture::setUp();

	// previous tests may have left a txn on the thread
	destroySpecific(TSS_KEY);
}

void TestTransactions::tearDown()
{
	txx_stop();
	TestFixture::tearDown();

	destroyEnv();
}

void TestTransactions::test_rclog()
{
	doOne();
}

void TestTransactions::test_basic()
{
	doTwo();
}

// sanity check
void TestTransactions::test_transactions()
{
	userlogc_debug("TestTransactions::test_transactions begin");
	BT_ASSERT_EQUAL(TX_OK, tx_open());
	BT_ASSERT_EQUAL(TX_OK, tx_begin());
	BT_ASSERT_EQUAL(TX_OK, tx_commit());
	BT_ASSERT_EQUAL(TX_OK, tx_close());

	
	userlogc("TestTransactions::test_transactions pass");
}

// check for protocol errors in a transactions lifecycle
void TestTransactions::test_protocol()
{
	userlogc_debug("TestTransactions::test_protocol begin");
	// should not be able to begin or complete a transaction before calling tx_open
	BT_ASSERT_EQUAL(TX_PROTOCOL_ERROR, tx_begin());
	BT_ASSERT_EQUAL(TX_PROTOCOL_ERROR, tx_commit());
	BT_ASSERT_EQUAL(TX_PROTOCOL_ERROR, tx_rollback());

	// tx close succeeds if was never opened
	BT_ASSERT_EQUAL(TX_OK, tx_close());

	// open should succeed
	BT_ASSERT_EQUAL(TX_OK, tx_open());
	// open second should should be idempotent
	BT_ASSERT_EQUAL(TX_OK, tx_open());
	// should not be able to complete a transaction before calling tx_begin
	BT_ASSERT(tx_commit() != TX_OK);
	BT_ASSERT(tx_rollback() != TX_OK);
	// should be able to close if a transaction hasn't been started
	BT_ASSERT_EQUAL(TX_OK, tx_close());

	// reopen the transaction - begin should succeed
	BT_ASSERT_EQUAL(TX_OK, tx_open());
	userlogc_debug("TestTransactions::test_protocol 2nd begin");
	BT_ASSERT_EQUAL(TX_OK, tx_begin());
	// should not be able to close a transaction before calling tx_commit or tx_rollback
	BT_ASSERT_EQUAL(TX_PROTOCOL_ERROR, tx_close());
	// rollback should succeed
	BT_ASSERT_EQUAL(TX_OK, tx_rollback());
	// should not be able to commit or rollback a transaction after it has already completed
	BT_ASSERT(tx_commit() != TX_OK);
	BT_ASSERT(tx_rollback() != TX_OK);

	// begin should succeed after terminating a transaction
	BT_ASSERT_EQUAL(TX_OK, tx_begin());
	BT_ASSERT_EQUAL(TX_OK, tx_commit());

	// close should succeed
	BT_ASSERT_EQUAL(TX_OK, tx_close());

	// tx_begin should return TX_PROTOCOL_ERROR if caller is already in transaction mode
	BT_ASSERT_EQUAL(TX_OK, tx_open());
	BT_ASSERT_EQUAL(TX_OK, tx_begin());
	BT_ASSERT_EQUAL(TX_PROTOCOL_ERROR, tx_begin());
	BT_ASSERT_EQUAL(TX_OK, tx_commit());
	BT_ASSERT_EQUAL(TX_OK, tx_close());

	BT_ASSERT_EQUAL(TX_OK, tx_open());
	/* cause RM 102 start to fail */
	fault_t fault = {0, 102, O_XA_START, XAER_RMERR};
	(void) dummy_rm_add_fault(&fault);
	// tx_begin should return TX_ERROR if rm return errors, and the caller is not in transaction mode
	BT_ASSERT_EQUAL(TX_ERROR, tx_begin());
	BT_ASSERT_EQUAL(TX_PROTOCOL_ERROR, tx_commit());
	/* cleanup */
	(void) dummy_rm_del_fault(fault.id);
	BT_ASSERT_EQUAL(TX_OK, tx_close());
	userlogc( (char*) "TestTransactions::test_protocol pass");
}

void TestTransactions::test_info()
{
	userlogc_debug("TestTransactions::test_info begin");
	BT_ASSERT_EQUAL(TX_OK, tx_open());
	BT_ASSERT_EQUAL(TX_OK, tx_begin());

	// verify that the initial values are correct
	// do not test for the initial value of info.when_return since it is implementation dependent
	// if the second parameter is 1 then test that we are in transaction mode
	CHECKINFO("initial values", 1, TX_COMMIT_DECISION_LOGGED, TX_UNCHAINED, 0l, TX_ACTIVE);
	BT_ASSERT_EQUAL(TX_OK, tx_commit());
	// if the second parameter is 0 then test that we are not in transaction mode
	CHECKINFO("not in tx context", 0, TX_COMMIT_DECISION_LOGGED, TX_UNCHAINED, 0l, -1l);

	(void) tx_set_commit_return(TX_COMMIT_COMPLETED);
	(void) tx_set_transaction_control(TX_CHAINED);
	(void) tx_set_transaction_timeout(10l);

	// begin another transaction
	BT_ASSERT_EQUAL(TX_OK, tx_begin());

	// verify that the new values are correct and that there is a running transaction
	CHECKINFO("modified values", 1, TX_COMMIT_COMPLETED, TX_CHAINED, 10l, TX_ACTIVE);
	// commit the transaction
	userlogc_debug("TestTransactions::test_info commit chained tx");
	BT_ASSERT_EQUAL(TX_OK, tx_commit());
	// transaction control mode is TX_CHAINED so there should be an active transaction after a commit
	CHECKINFO("TX_CHAINED after commit", 1, TX_COMMIT_COMPLETED, TX_CHAINED, 10l, TX_ACTIVE);

	// rollback the chained transaction
	BT_ASSERT_EQUAL(TX_OK, tx_rollback());
	// transaction control mode is TX_CHAINED so there should be an active transaction after a rollback
	CHECKINFO("XX", 1, TX_COMMIT_COMPLETED, TX_CHAINED, 10l, TX_ACTIVE);

	// stop chaining transactions
	(void) tx_set_transaction_control(TX_UNCHAINED);
	BT_ASSERT_EQUAL(TX_OK, tx_rollback());
	// transaction control mode should now be TX_UNCHAINED so there should not be an active transaction after a rollback
	CHECKINFO("TX_UNCHAINED after rollback", 0, TX_COMMIT_COMPLETED, TX_UNCHAINED, 10l, -1l);

	BT_ASSERT_EQUAL(TX_OK, tx_close());

	// If info is null, no TXINFO structure is returned
	BT_ASSERT_EQUAL(TX_PROTOCOL_ERROR, tx_info(NULL));
	userlogc( (char*) "TestTransactions::test_info pass");
}

// test for transaction timeout behaviour
void TestTransactions::test_timeout()
{
	long timeout = 4;
	long delay = 8;
	userlogc_debug("TestTransactions::test_timeout begin");
	// cause RMs to sleep during 2PC
	fault_t fault1 = {0, 102, O_XA_COMMIT, XA_OK, F_DELAY, (void*)&delay};
	fault_t fault2 = {0, 100, O_XA_PREPARE, XA_OK, F_DELAY, (void*)&delay};

	BT_ASSERT_EQUAL(TX_OK, tx_open());

	BT_ASSERT_EQUAL(TX_OK, tx_begin());

	// set timeout - the value should not have any effect until the next call to tx_begin
	BT_ASSERT_EQUAL(TX_OK, tx_set_transaction_timeout(timeout));
	doThree(delay);
	BT_ASSERT_EQUAL(TX_OK, tx_commit());

	// start another transaction
	BT_ASSERT_EQUAL(TX_OK, tx_begin());
	// sleep for longer than the timeout
	userlogc_debug("TestTransactions::test_timeout sleeping");
	doSix(delay);
	userlogc_debug("TestTransactions::test_timeout testing for rollback on commit");
	BT_ASSERT_EQUAL(TX_ROLLBACK, tx_commit());

	// cause the RM to delay for delay seconds during commit processing
	(void) dummy_rm_add_fault(&fault1);
	(void) dummy_rm_add_fault(&fault2);
	userlogc_debug("TestTransactions::test_timeout injecting delay after phase 1");
	BT_ASSERT_EQUAL(TX_OK, tx_begin());
	// once the transaction has started 2PC any further delays (beyond the timeout period) should have no effect
	userlogc_debug("TestTransactions::test_timeout validating that the delay was ignored");
	BT_ASSERT_EQUAL(TX_OK, tx_commit());

	/* cleanup */
	(void) dummy_rm_del_fault(fault1.id);
	(void) dummy_rm_del_fault(fault2.id);
	BT_ASSERT_EQUAL(TX_OK, tx_close());
	userlogc( (char*) "TestTransactions::test_timeout pass");
}

void TestTransactions::test_rollback()
{
	// TODO check the behaviour when a real RM is used.
	userlogc_debug("TestTransactions::test_rollback begin");

	fault_t fault1 = {0, 102, O_XA_COMMIT, XA_HEURHAZ};
	/* cause RM 102 start to fail */
	fault_t fault2 = {0, 102, O_XA_START, XAER_RMERR};

	BT_ASSERT_EQUAL(TX_OK, tx_open());
	BT_ASSERT_EQUAL(TX_OK, tx_begin());
	doFour();
	CHECKINFO("set_rollback_only", 1, TX_COMMIT_DECISION_LOGGED, TX_UNCHAINED, 0l, TX_ROLLBACK_ONLY);
	BT_ASSERT_EQUAL(TX_ROLLBACK, tx_commit());

	BT_ASSERT_EQUAL(TX_OK, tx_set_transaction_control(TX_CHAINED));
	BT_ASSERT_EQUAL(TX_OK, tx_begin());
	(void) dummy_rm_add_fault(&fault2);
	doFour();
	BT_ASSERT_EQUAL(TX_ROLLBACK_NO_BEGIN, tx_commit());
	(void) dummy_rm_del_fault(fault2.id);

	BT_ASSERT_EQUAL(TX_OK, tx_set_commit_return(TX_COMMIT_COMPLETED));
	BT_ASSERT_EQUAL(TX_OK, tx_begin());
	(void) dummy_rm_add_fault(&fault1);
	(void) dummy_rm_add_fault(&fault2);
	BT_ASSERT_EQUAL(TX_HAZARD_NO_BEGIN, tx_commit());

	(void) dummy_rm_del_fault(fault1.id);
	(void) dummy_rm_del_fault(fault2.id);
	BT_ASSERT_EQUAL(TX_OK, tx_close());
	userlogc( (char*) "TestTransactions::test_rollback pass");
}

void TestTransactions::test_RM()
{
	/* cause RM 102 to generate a mixed heuristic */
	fault_t fault1 = {0, 102, O_XA_COMMIT, XA_HEURMIX};
	/* cause RM 102 start to fail */
	fault_t fault2 = {0, 102, O_XA_START, XAER_RMERR};

	userlogc_debug("TestTransactions::test_RM begin");
	/* inject a commit fault in Resource Manager with rmid 102 */
	(void) dummy_rm_add_fault(&fault1);

	BT_ASSERT_EQUAL(TX_OK, tx_open());
	/* turn on heuristic reporting (ie the commit does not return until 2PC is complete) */
	BT_ASSERT_EQUAL(TX_OK, tx_set_commit_return(TX_COMMIT_COMPLETED));
	BT_ASSERT_EQUAL(TX_OK, tx_begin());
	/* since we have added a XA_HEURMIX fault tx_commit should return an mixed error */
	userlogc_debug("TestTransactions::test_RM expecting TX_MIXED");
	BT_ASSERT_EQUAL(TX_MIXED, tx_commit());

	/*
	 * repeat the test but with chained transactions and heuristic reporting enabled
	 */
	BT_ASSERT_EQUAL(TX_OK, tx_set_transaction_control(TX_CHAINED));
	BT_ASSERT_EQUAL(TX_OK, tx_set_commit_return(TX_COMMIT_COMPLETED));
	BT_ASSERT_EQUAL(TX_OK, tx_begin());

	/* inject a fault that will cause the chained tx_begin to fail */
	(void) dummy_rm_add_fault(&fault2);
	/*
	 * commit should fail with a heuristic and the attempt to start a chained transaction should fail
	 * since we have just injected a start fault
	 */
	BT_ASSERT_EQUAL(TX_MIXED_NO_BEGIN, tx_commit());

	/* clean up */
	(void) dummy_rm_del_fault(fault1.id);
	(void) dummy_rm_del_fault(fault2.id);

	/* should still be able to clean up after failing to commit a chained transaction */
	BT_ASSERT_EQUAL(TX_OK, tx_close());
	userlogc( (char*) "TestTransactions::test_RM pass");
}

/**
 * Test that XIDs are recovered via the XA spec xa_recover method.
 * This functionality covers the following failure scenario:
 * - server calls prepare on a RM
 * - RM prepares but the the server fails before it can write to its transaction recovery log
 * In this case the RM will have a pending transaction branch which does not appear in
 * the recovery log. Calling xa_recover on the RM will return the 'missing' XID which the
 * recovery scan will replay.
 */
void TestTransactions::test_RM_recovery_scan()
{
	long nbranches = 2l;
	fault_t fault1 = {0, 102, O_XA_RECOVER, XA_OK, F_ADD_XIDS, &nbranches, 0};

	userlogc_debug("TestTransactions::test_RM_recovery_scan begin");

	/* tell the Resource Manager with rmid 102 to remember prepared XID's */
	(void) dummy_rm_add_fault(&fault1);

	/* tx_open() should trigger a recovery scan (see XAResourceManagerFactory::run_recovery() */
	BT_ASSERT_EQUAL(TX_OK, tx_open());

	// all resources should have been recovered
	BT_ASSERT_EQUAL(nbranches, (long)fault1.res);
	// and the number that were recovered should also be nbranches
	BT_ASSERT_EQUAL(nbranches, (long)fault1.res2);

//	atmibroker::tx::TxManager::get_instance()->getRMFac().run_recovery();

	/* clean up */
	(void) dummy_rm_del_fault(fault1.id);
	BT_ASSERT_EQUAL(TX_OK, tx_close());
	userlogc("TestTransactions::test_RM_recovery_scan pass");
}

/*
 * Test whether enlisting a resource with a remote transaction manager works
 * This test attempts to simulate what XAResourceManager does.
 * The real test for interacting with resource managers happens as a side effect
 * of begining and completing a transactions provided some Resouce Managers
 * have been configured in btconfig.xml
 */
void TestTransactions::test_register_resource()
{
	userlogc_debug("TestTransactions::test_register_resource begin");
	// start a transaction running
	BT_ASSERT_EQUAL(TX_OK, tx_open());
	BT_ASSERT_EQUAL(TX_OK, tx_begin());

	void* ra = doFive();

	// commit the transaction
	BT_ASSERT_EQUAL(TX_OK, tx_commit());
	doSeven(ra);
	// clean up
	BT_ASSERT_EQUAL(TX_OK, tx_close());
	userlogc("TestTransactions::test_register_resource pass");
}

/*
 * Test tx_set_commit_return(), tx_set_transaction_control(), tx_set_transaction_timeout()
 */
void TestTransactions::test_tx_set()
{
	userlogc("TestTransactions::test_tx_set begin");
	// tx_set_* return TX_PROTOCOL_ERROR if not call tx_open
	BT_ASSERT_EQUAL(TX_PROTOCOL_ERROR, tx_set_transaction_control(TX_CHAINED));
	BT_ASSERT_EQUAL(TX_PROTOCOL_ERROR, tx_set_commit_return(TX_COMMIT_COMPLETED));
	BT_ASSERT_EQUAL(TX_PROTOCOL_ERROR, tx_set_transaction_timeout(10l));
	
	BT_ASSERT_EQUAL(TX_OK, tx_open());
	BT_ASSERT_EQUAL(TX_EINVAL, tx_set_transaction_control(2l));
	BT_ASSERT_EQUAL(TX_EINVAL, tx_set_commit_return(2l));
	BT_ASSERT_EQUAL(TX_EINVAL, tx_set_transaction_timeout(-1l));
	BT_ASSERT_EQUAL(TX_OK, tx_close());
	userlogc("TestTransactions::test_tx_set pass");
}
