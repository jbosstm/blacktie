/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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
#include "tx.h"
#include "ThreadLocalStorage.h"
#include "XAResourceAdaptorImpl.h"

// includes for looking up orbs
#include "tao/ORB_Core.h"
#include "tao/ORB_Table.h"
#include "tao/ORB_Core_Auto_Ptr.h"
#include "OrbManagement.h"

// sanity check
void TestTransactions::test_transactions() {
	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);
	CPPUNIT_ASSERT(tx_commit() == TX_OK);
	CPPUNIT_ASSERT(tx_close() == TX_OK);
}

// check for protocol errors in a transactions lifecycle
void TestTransactions::test_protocol() {
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

static CORBA::ORB_ptr find_orb(const char * name) {
	TAO::ORB_Table * const orb_table = TAO::ORB_Table::instance();
	::TAO_ORB_Core* oc = orb_table->find(name);

	return (oc == 0 ? NULL : oc->orb());
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
        0 /* char data[XIDDATASIZE]; */
};


/*
 * Test whether enlisting a resource with a remote transaction manager works
 * This test attempts to simulate what XAResourceManager does.
 * The real test for interacting with resource managers happens as a side effect
 * of begining and completing a transactions provided some Resouce Managers
 * have been configured in Environment.xml
 */
void TestTransactions::test_register_resource() {
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
