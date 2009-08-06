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

#include "BaseServerTest.h"
#include "XATMITestSuite.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_string.h"

#include "xatmi.h"
#include "tx.h"

#include "tx/TestTxRMTPCall.h"

extern "C" {
#include "tx/products.h"
}

extern void tx_db_service(TPSVCINFO *svcinfo);

static char *TX_RM_SVC = (char *) "tpcall_x_octet";

static int cnt = 0;
//static product_t prods[8];
static product_t *prods = products;
static char testid[16];
static const char *emps[] = {"8000", "8001", "8002", "8003", "8004", "8005", "8006", "8007"};

static void set_test_id(const char *id) {
	(void) strncpy(testid, id, sizeof (testid));
}

static int is_tx_in_state(enum TX_TYPE txtype) {
	TXINFO txinfo;
	int rv = tx_info(&txinfo);
	int ts = (txtype == TX_TYPE_NONE ? -1 : TX_ACTIVE);

	userlogc( (char*) "validating tx status actual %d vrs desired %d", txinfo.transaction_state, ts);

	return (txinfo.transaction_state == ts);
}

static int send_req(test_req_t *req, char **prbuf) {
	long rsz = sizeof (test_req_t);
	long callflags = 0L;
	test_req_t *resp;
	int rv = 0;

	resp = (test_req_t *) tpalloc((char*) "X_C_TYPE", (char*) "dc_buf", sizeof (test_req_t));

	userlogc( (char*) "Invoke Service %s %4d: prod=%d op=%c data=%s dbf=%s tx=%d", TX_RM_SVC,
		req->id, req->prod, req->op, req->data, req->db, req->txtype);
	if (tpcall(TX_RM_SVC, (char *) req, sizeof (test_req_t), (char **) &resp, &rsz, callflags) == -1) {
		userlogc( (char*) "TP ERROR tperrno: %d", tperrno);
		rv = -1;
	} else if (prbuf && *prbuf) {
		strncpy(*prbuf, resp->data, sizeof (resp->data));
		rv = resp->status;
	} else {
		rv = resp->status;
	}

	tpfree((char *) resp);

	return rv;
}

static int count_records(const char *msg, const char *key, int in_tx, int expect) {
	int cnt = -1;

	if (in_tx || start_tx(TX_TYPE_BEGIN) == 0) {
		int rv = 0;
		test_req_t *req;
		test_req_t res;
		product_t *p = prods;
		char *rbuf = (char *) (res.data);

		for (p = prods; p->id != -1; p++) {
			int remote = LOCAL_ACCESS;

			if ((p->loc & remote) == 0) /* the RM does not support the requested access type */
				remote = p->loc;

			req = get_buf((remote & REMOTE_ACCESS), key, p->dbname, '1', p->id, TX_TYPE_NONE, expect);
			userlogc( "invoke prod %s (id=%d) remote=%d dbf=%s", p->pname, p->id, remote, p->dbname);

			rv = ((remote & REMOTE_ACCESS) ? send_req(req, &rbuf) : p->access(req, &res));
userlogc( "invoked ok");

			if (rv)
				userlogc( "BAD REQ %d", rv);

			free_buf(remote, req);
			rv = (rv == 0 ? atoi(res.data) : -1);

userlogc( "and count is %d", rv);
			if (rv == -1) {
				userlogc( "Error: Db %d access error", p->id);
				return -1;
			}
			if (rv != cnt && cnt != -1) {
				userlogc( "All databases should have the same no of records: db %d cnt %d (prev was %d)", p->id, rv, cnt);
				return -1;
			}

			cnt = rv;
		}

		if (in_tx || end_tx(TX_TYPE_COMMIT) == 0)
			return cnt;
	}

	return cnt;
}

static int check_count(const char *msg, const char *key, int in_tx, int expect) {
	int rcnt;

	if ((rcnt = count_records(msg, key, 0, expect)) != expect) {
		userlogc( "WRONG NUMBER OF RECORDS: %d expected %d", rcnt, expect);
		return -1;
	}

	userlogc( "%s: RECORD COUNT: %d expected %d", testid, rcnt, expect);
	return 0;
}

static int db_op(const char *msg, const char *data, char op, enum TX_TYPE txtype,
				 char **prbuf, int remote, int migrating, int expect) {
	if (msg)
		userlogc( (char *) "%s: %s %s", testid, ((remote | REMOTE_ACCESS) ? "REMOTE" : "LOCAL"), msg);

	if (start_tx(txtype) == 0) {
		int rv = 0;
		test_req_t *req;
		test_req_t res;
		product_t *p = prods;

		for (p = prods; p->id != -1; p++) {
#if 0
			if (migrating && (p->xaflags() & TMNOMIGRATE)) {
				/* the RM does not support tx migration (see XA spec for explanation */
				userlogc( "Info: RM %d does not support tx migration (switching from remote)", p->id);
				remote = !remote;
			}
#endif

			if ((p->loc & remote) == 0) /* the RM does not support the requested access type */
				remote = p->loc;

			req = get_buf((remote & REMOTE_ACCESS), data, p->dbname, op, p->id, txtype, expect);
			userlogc( "invoke prod %s (id=%d) remote=0x%x isrem=%d dbf=%s tx=%d",
				p->pname, p->id, remote, (remote | REMOTE_ACCESS), p->dbname, req->txtype);

			rv = ((remote & REMOTE_ACCESS) ? send_req(req, prbuf) : p->access(req, &res));

			if (rv)
				userlogc( "BAD REQ %d", rv);

			free_buf(remote, req);
		}

		if (end_tx(txtype) == 0)
			return rv;
	}

	return -1;
}

static void init() {
	userlogc((char*) "TestTxRMTPCall::init");
	int rv;

	userlogc((char*) "RM_test - advertising %s", TX_RM_SVC);
	tpadvertise(TX_RM_SVC, tx_db_service);

	if ((rv = tx_open()) != TX_OK)
		userlogc((char*) "tx_open error %d", rv);

	CPPUNIT_ASSERT_MESSAGE("tx_open error", rv == 0);

	/* start off with no records */
	if ((rv = db_op("DELETE AT SETUP", emps[0], '3', TX_TYPE_BEGIN_COMMIT, 0, LOCAL_ACCESS, 0, -1)) != 0)
		userlogc((char*) "DELETE error %d", rv);

	CPPUNIT_ASSERT_MESSAGE("DELETE AT SETUP error", rv == 0);

	if ((rv = check_count("COUNT RECORDS", emps[0], 0, 0)))
		userlogc((char*) "COUNT AT SETUP - WRONG number of records %d", rv);

	CPPUNIT_ASSERT_MESSAGE("COUNT AT SETUP error", rv == 0);
}

static void fini() {
	int rv;

	userlogc((char*) "TestTxRMTPCall::tearDown");

	/* delete records starting from emps[0], */
	if ((rv = db_op("DELETE", emps[0], '3', TX_TYPE_BEGIN_COMMIT, 0, LOCAL_ACCESS, 0, -1)))
		userlogc((char*) "DELETE AT TEARDOWN error %d", rv);

	CPPUNIT_ASSERT_MESSAGE("DELETE AT TEARDOWN error", rv == 0);
	cnt = 0;
	if ((rv = check_count("COUNT RECORDS", emps[0], 0, cnt)))
		userlogc((char*) "COUNT AT TEARDOWN - WRONG number of records %d", rv);

	CPPUNIT_ASSERT_MESSAGE("COUNT AT TEARDOWN error", rv == 0);

	if ((rv = tx_close()) != TX_OK)
		userlogc((char*) "tx_close error %d", rv);

	CPPUNIT_ASSERT_MESSAGE("tx_close error", rv == 0);

	(void) tpunadvertise(TX_RM_SVC);
}

void TestTxRMTPCall::setUp() {
	userlogc((char*) "TestTxRMTPCall::setUp");
#if 0
    int rv1 = mkdir("db1");
    int rv2 = mkdir("db2");

    if (rv1 != 0)
        logit(0, (char *) "unable to crate database dir %s error %d", "db1", rv1);
    if (rv2 != 0)
        logit(0, (char *) "unable to crate database dir %s error %d", "db2", rv1);
#endif
	BaseServerTest::setUp();
}

void TestTxRMTPCall::tearDown() {
	userlogc((char*) "TestTxRMTPCall::tearDown");
	// Clean up server
	BaseServerTest::tearDown();
}

/**
 * start a transaction
 * perform a remote insert on each target db
 * perform another remote insert on each target db
 * perform a local insert on each target db
 * commit the transaction
 *
 * Note for databases that do not support tx migration (meaning that performing a
 * remote insert followed by a local insert is not supported) the local operation
 * is switched to a remote one (which ensures that all the updates are done in the
 * same thread).
 */
void TestTxRMTPCall::test1() {
	int rv;
return;
	init();
	set_test_id("Test 1");
	/* ask the remote service to insert a record */
	if ((rv = db_op("INSERT 1", emps[5], '0', TX_TYPE_BEGIN, 0, REMOTE_ACCESS, 0, -1)))
		userlogc((char*) "INSERT 1 error %d", rv);

	CPPUNIT_ASSERT_EQUAL(rv, 0);

	/* ask the remote service to insert another record in the same transaction */
	if ((rv = db_op("INSERT 2", emps[6], '0', TX_TYPE_NONE, 0, REMOTE_ACCESS, 0, -1)))
		userlogc((char*) "INSERT 2 error %d", rv);

	CPPUNIT_ASSERT_EQUAL(rv, 0);

	/* insert a record and end the already running transaction */
	if ((rv = db_op("INSERT 3", emps[7], '0', TX_TYPE_COMMIT, 0, LOCAL_ACCESS, 1, -1)))
		userlogc((char*) "INSERT 3 error %d", rv);

	CPPUNIT_ASSERT_EQUAL(rv, 0);

	cnt += 3;

	/* make sure the record count increases by 3 */
	if ((rv = check_count("COUNT RECORDS", emps[0], 0, cnt)))
		userlogc((char*) "COUNT RECORDS error %d", rv);

	CPPUNIT_ASSERT_EQUAL(rv, 0);
	fini();
}

void TestTxRMTPCall::test2() {
	int pid = 2;
	int rv = -1;

	userlogc((char*) "TestTxRMTPCall::test2");
	tpadvertise(TX_RM_SVC, tx_db_service);

	test_req_t *req = get_buf(1, "", "", '1', pid, TX_TYPE_BEGIN_COMMIT, 0);
	test_req_t *res = (test_req_t *) tpalloc((char*) "X_C_TYPE", (char*) "dc_buf", sizeof (test_req_t));
	char *rbuf = (char *) (res->data);
	userlogc((char*) "TestTxRMTPCall::test2 tx=%d", req->txtype);

	if ((rv = tx_open()) != TX_OK) {
		userlogc((char*) "tx_open error %d", rv);
	} else if (start_tx(req->txtype) == 0) {
		rv = send_req(req, &rbuf);
		if (end_tx(req->txtype) != 0)
			userlogc((char*) "TestTxRMTPCall::test2 end tx failed");

		if ((rv = tx_close()) != TX_OK)
			userlogc((char*) "tx_close error %d", rv);
	}

	free_buf(1, req);
	::tpfree((char *) res);

	CPPUNIT_ASSERT(rv == 0);
}

void tx_db_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "service %s running", TX_RM_SVC);

	test_req_t *req = (test_req_t *) svcinfo->data;
	test_req_t *resp = (test_req_t *) tpalloc((char*) "X_C_TYPE", (char*) "dc_buf", sizeof (test_req_t));
	product_t *p = products;

	resp->status = -1;

	if (!is_tx_in_state(req->txtype)) {
		userlogc((char*) "TRANSACTION not in expected state");
	} else {
		ACE_OS::snprintf(resp->data, sizeof (resp->data), "Unsupported database: %s", req->db);
		for (p = products; p->id != -1; p++) {
			if (req->prod == p->id) {
				strncpy(req->db, p->dbname, sizeof(req->db));
				userlogc((char*) "Service %s %4d: prod=%8s (id=%d) op=%c tx=0x%x data=%s", TX_RM_SVC,
					req->id, p->pname, p->id, req->op, req->txtype, req->data);
				resp->status = p->access(req, resp);
				userlogc((char*) "Service %s %4d: resp->status=%d", TX_RM_SVC, req->id, resp->status);
				break;
			}
		}
	}

	tpreturn(1, 1, (char *) resp, sizeof (test_req_t), 0);
}
