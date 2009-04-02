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
#include <cppunit/extensions/HelperMacros.h>
#include "TestTransactions.h"
#include "OrbManagement.h"
#include "txClient.h"
#include "testrm.h"
#include "ThreadLocalStorage.h"
#include "XAResourceAdaptorImpl.h"
#include "ace/OS_NS_unistd.h"

// sanity check
void TestTransactions::test_transactions()
{
	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);
	CPPUNIT_ASSERT(tx_commit() == TX_OK);
	CPPUNIT_ASSERT(tx_close() == TX_OK);
}

// check for protocol errors in a transactions lifecycle
void TestTransactions::test_protocol()
{
	// should not be able to begin or complete a transaction before calling tx_open
	CPPUNIT_ASSERT(tx_begin() == TX_PROTOCOL_ERROR);
	CPPUNIT_ASSERT(tx_commit() == TX_PROTOCOL_ERROR);
	CPPUNIT_ASSERT(tx_rollback() == TX_PROTOCOL_ERROR);

	// tx close succeeds if was never opened
	CPPUNIT_ASSERT(tx_close() == TX_OK);

	// open should succeed
	CPPUNIT_ASSERT(tx_open() == TX_OK);
	// open second should should be idempotent
	CPPUNIT_ASSERT(tx_open() == TX_OK);
	// should not be able to complete a transaction before calling tx_begin
	CPPUNIT_ASSERT(tx_commit() != TX_OK);
	CPPUNIT_ASSERT(tx_rollback() != TX_OK);
	// should be able to close if a transaction hasn't been started
	CPPUNIT_ASSERT(tx_close() == TX_OK);

	// reopen the transaction - begin should succeed
	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);
	// should not be able to close a transaction before calling tx_commit or tx_rollback
	CPPUNIT_ASSERT(tx_close() != TX_OK);
	// rollback should succeed
	CPPUNIT_ASSERT(tx_rollback() == TX_OK);
	// should not be able to commit or rollback a transaction after it has already completed
	CPPUNIT_ASSERT(tx_commit() != TX_OK);
	CPPUNIT_ASSERT(tx_rollback() != TX_OK);

	// begin should succeed after terminating a transaction
	CPPUNIT_ASSERT(tx_begin() == TX_OK);
	CPPUNIT_ASSERT(tx_commit() == TX_OK);

	// close should succeed
	CPPUNIT_ASSERT(tx_close() == TX_OK);
}

static void check_info(const char *msg, int rv,
	COMMIT_RETURN cr, TRANSACTION_CONTROL tc, TRANSACTION_TIMEOUT tt, TX_TRANSACTION_STATE ts)
{
	TXINFO txinfo;

	CPPUNIT_ASSERT(tx_info(&txinfo) == rv);

	if (cr >= 0) CPPUNIT_ASSERT_MESSAGE(msg, txinfo.when_return == cr);
	if (tc >= 0) CPPUNIT_ASSERT_MESSAGE(msg, txinfo.transaction_control == tc);
	if (tt >= 0) CPPUNIT_ASSERT_MESSAGE(msg, txinfo.transaction_timeout == tt);
	if (ts >= 0) CPPUNIT_ASSERT_MESSAGE(msg, txinfo.transaction_state == ts);
}

void TestTransactions::test_info()
{
	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);

	// verify that the initial values are correct
	// do not test for the initial value of info.when_return since it is implementation dependent
	// if the second parameter is 1 then test that we are in transaction mode
	check_info("initial values", 1, -1L, TX_UNCHAINED, 0L, TX_ACTIVE);
	CPPUNIT_ASSERT(tx_commit() == TX_OK);
	// if the second parameter is 0 then test that we are not in transaction mode
	check_info("not in tx context", 0, -1L, TX_UNCHAINED, 0L, -1);

	(void) tx_set_commit_return(TX_COMMIT_COMPLETED);
	(void) tx_set_transaction_control(TX_CHAINED);
	(void) tx_set_transaction_timeout(10);

	// begin another transaction
	CPPUNIT_ASSERT(tx_begin() == TX_OK);

	// verify that the new values are correct and that there is a running transaction
	check_info("modified values", 1, TX_COMMIT_COMPLETED, TX_CHAINED, 10L, TX_ACTIVE);
	// commit the transaction
	CPPUNIT_ASSERT(tx_commit() == TX_OK);
	// transaction control mode is TX_CHAINED so there should be an active transaction after a commit
	check_info("TX_CHAINED after commit", 1, TX_COMMIT_COMPLETED, TX_CHAINED, 10L, TX_ACTIVE);

	// rollback the chained transaction
	CPPUNIT_ASSERT(tx_rollback() == TX_OK);
	// transaction control mode is TX_CHAINED so there should be an active transaction after a rollback
	check_info("XX", 1, TX_COMMIT_COMPLETED, TX_CHAINED, 10L, TX_ACTIVE);

	// stop chaining transactions
	(void) tx_set_transaction_control(TX_UNCHAINED);
	CPPUNIT_ASSERT(tx_rollback() == TX_OK);
	// transaction control mode should now be TX_UNCHAINED so there should not be an active transaction after a rollback
	check_info("TX_UNCHAINED after rollback", 0, TX_COMMIT_COMPLETED, TX_UNCHAINED, 10L, -1);

	CPPUNIT_ASSERT(tx_close() == TX_OK);
}

// test for transaction timeout behaviour
void TestTransactions::test_timeout()
{
	long timeout = 1;
	long delay = 2;
	// cause RMs to sleep during 2PC
	fault_t fault1 = {0, 102, O_XA_COMMIT, XA_OK, F_DELAY, (void*)&delay};
	fault_t fault2 = {0, 100, O_XA_PREPARE, XA_OK, F_DELAY, (void*)&delay};

	CPPUNIT_ASSERT(tx_open() == TX_OK);

	CPPUNIT_ASSERT(tx_begin() == TX_OK);

	// set timeout - the value should not have any effect until the next call to tx_begin
	CPPUNIT_ASSERT(tx_set_transaction_timeout(timeout) == TX_OK);
	(void) ACE_OS::sleep(delay);
	CPPUNIT_ASSERT(tx_commit() == TX_OK);

	// start another transaction
	CPPUNIT_ASSERT(tx_begin() == TX_OK);
	// sleep for longer than the timeout
	(void) ACE_OS::sleep(delay);
	CPPUNIT_ASSERT(tx_commit() == TX_ROLLBACK);

	// cause the RM to delay for delay seconds during commit processing
	(void) dummy_rm_add_fault(&fault1);
	(void) dummy_rm_add_fault(&fault2);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);
	// once the transaction has started 2PC any further delays (beyond the timeout period) should have no effect
	CPPUNIT_ASSERT(tx_commit() == TX_OK);

	/* cleanup */
	(void) dummy_rm_del_fault(fault1.id);
	(void) dummy_rm_del_fault(fault2.id);
	CPPUNIT_ASSERT(tx_close() == TX_OK);
}

void TestTransactions::test_RM()
{
	/* cause RM 102 to generate a mixed heuristic */
	fault_t fault1 = {0, 102, O_XA_COMMIT, XA_HEURMIX};
	/* cause RM 102 start to fail */
	fault_t fault2 = {0, 102, O_XA_START, XAER_RMERR};

	/* inject a commit fault in Resource Manager with rmid 102 */
	(void) dummy_rm_add_fault(&fault1);

	CPPUNIT_ASSERT(tx_open() == TX_OK);
	/* turn on heuristic reporting (ie the commit does not return until 2PC is complete) */
	CPPUNIT_ASSERT(tx_set_commit_return(TX_COMMIT_COMPLETED) == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);
	/* since we have added a XA_HEURMIX fault tx_commit should return an mixed error */
	CPPUNIT_ASSERT(tx_commit() == TX_MIXED);

	/*
	 * repeat the test but with chained transactions and heuristic reporting enabled
	 */
	CPPUNIT_ASSERT(tx_set_transaction_control(TX_CHAINED) == TX_OK);
	CPPUNIT_ASSERT(tx_set_commit_return(TX_COMMIT_COMPLETED) == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);

	/* inject a fault that will cause the chained tx_begin to fail */
	(void) dummy_rm_add_fault(&fault2);
	/*
	 * commit should fail with a heuristic and the attempt to start a chained transaction should fail
	 * since we have just injected a start fault
	 */
	CPPUNIT_ASSERT(tx_commit() == TX_MIXED_NO_BEGIN);

	/* clean up */
	(void) dummy_rm_del_fault(fault1.id);
	(void) dummy_rm_del_fault(fault2.id);

	/* should still be able to clean up after failing to commit a chained transaction */
	CPPUNIT_ASSERT(tx_close() == TX_OK);
}

static int fn1(char *a, int i, long l) { return 0; }
static int fn2(XID *x, int i, long l) { return 0; }
static int fn3(XID *, long l1, int i, long l2) { return 0; }
static int fn4(int *ip1, int *ip2, int i, long l) { return 0; }
static struct xa_switch_t real_resource = { "DummyRM", 0L, 0, fn1, fn1, /* open and close */
	fn2, fn2, fn2, fn2, fn2, /*start, end, rollback, prepare, commit */
	fn3, /* recover */
	fn2, /* forget */
	fn4 /* complete */
};
// manufacture a dummy RM transaction id
static XID xid = {
        1L, /* long formatID */
        0L, /* long gtrid_length */
        0L, /* long bqual_length */
        {0} /* char data[XIDDATASIZE]; */
};


/*
 * Test whether enlisting a resource with a remote transaction manager works
 * This test attempts to simulate what XAResourceManager does.
 * The real test for interacting with resource managers happens as a side effect
 * of begining and completing a transactions provided some Resouce Managers
 * have been configured in Environment.xml
 */
void TestTransactions::test_register_resource()
{
	// start a transaction running
	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);

	CosTransactions::Control_ptr curr = (CosTransactions::Control_ptr) getSpecific(TSS_KEY);
	// there should be a transaction running
	CPPUNIT_ASSERT(!CORBA::is_nil(curr));
	CosTransactions::Coordinator_ptr c = curr->get_coordinator();
	// and it should have a coordinator
	CPPUNIT_ASSERT(!CORBA::is_nil(c));

	// a side effect of starting a transaction is to start an orb
	CORBA::ORB_ptr orb = find_orb("ots");
	CPPUNIT_ASSERT(!CORBA::is_nil(orb));
	// get a handle on a poa
	CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

	PortableServer::POAManager_var mgr = poa->the_POAManager();
	mgr->activate();

	// now for the real test:
	// - create a CosTransactions::Resource ...
	//XAResourceAdaptorImpl * ra = new XAResourceAdaptorImpl(findConnection("ots"), "Dummy", "", "", 123L, &real_resource);
	XAResourceAdaptorImpl * ra = new XAResourceAdaptorImpl(NULL, &xid, 123L, &real_resource);
	//XAResourceAdaptorImpl * ra = new XAResourceAdaptorImpl(123L, &real_resource);
	CORBA::Object_ptr ref = poa->servant_to_reference(ra);
	CosTransactions::Resource_var v = CosTransactions::Resource::_narrow(ref);

	try {
		// ... and enlist it with the transaction
		CosTransactions::RecoveryCoordinator_ptr rc = c->register_resource(v);
		CPPUNIT_ASSERT(!CORBA::is_nil(rc));
	} catch (CosTransactions::Inactive&) {
		CPPUNIT_ASSERT(false);
	} catch (const CORBA::SystemException& ex) {
		ex._tao_print_exception("Resource registration error: ");
		CPPUNIT_ASSERT(false);
	}

	// commit the transaction
	CPPUNIT_ASSERT(tx_commit() == TX_OK);
	// the resource should have been committed
	CPPUNIT_ASSERT(ra->is_complete());
	// clean up
	CPPUNIT_ASSERT(tx_close() == TX_OK);
}
