/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat, Inc., and others contributors as indicated
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xatmi.h"
#include "tx.h"

#include "userlogc.h"
#include "client.h"

extern int DISABLE_XA;

static enum DB_TYPE prod;
static char *emps[] = {"8000", "8001", "8002", "8003", "8004", "8005", "8006", "8007"};
#define DBF "db1" /* backing file for database 1 */

static int is_begin(enum TX_TYPE txtype) {
	return (txtype == TX_TYPE_BEGIN || txtype == TX_TYPE_BEGIN_COMMIT || txtype == TX_TYPE_BEGIN_ABORT);
}
static int is_commit(enum TX_TYPE txtype) {
	return (txtype == TX_TYPE_COMMIT || txtype == TX_TYPE_BEGIN_COMMIT);
}
static int is_abort(enum TX_TYPE txtype) {
	return (txtype == TX_TYPE_ABORT || txtype == TX_TYPE_BEGIN_ABORT);
}
static int start_tx(enum TX_TYPE txtype) {
	if (is_begin(txtype)) {
		userlogc((char*) "- Starting Transaction");
		if (tx_begin() != TX_OK) {
			userlogc((char*) "TX ERROR - Could not begin transaction: ");
			return -1;
		}
	}

	return 0;
}
static int end_tx(enum TX_TYPE txtype) {
	int rv = 0;
	if (is_commit(txtype)) {
		userlogc((char*) "- Commiting transaction");
		if (tx_commit() != TX_OK)
			rv = -1;
	} else if (is_abort(txtype)) {
		userlogc((char*) "- Rolling back transaction");
		if (tx_rollback() != TX_OK)
			rv = -1;
	}

	if (rv != 0)
		userlogc((char*) "TX ERROR - Could not terminate transaction");

	return rv;
}

static int send_req(enum DB_TYPE ptype, const char * msg, const char *dbfile, const char *data, char op, char **prbuf) {
	long rsz = sizeof (test_req_t);
	long callflags = 0L;
	test_req_t *req;
	test_req_t *resp;
	int rv = 0;

	resp = (test_req_t *) tpalloc((char*) "X_C_TYPE", (char*) "dc_buf", sizeof (test_req_t));
	req = get_tbuf(data, dbfile, op, ptype, 0);

	userlogc((char*) "Invoke Service DBS: prod=%d op=%c data=%s dbf=%s", req->prod, req->op, req->data, req->db);
	if (tpcall("DBS", (char *) req, sizeof (test_req_t), (char **) &resp, &rsz, callflags) == -1) {
		userlogc((char*) "TP ERROR tperrno: %d", tperrno);
		rv = -1;
	}

	tpfree((char *) req);
	tpfree((char *) resp);

	return rv;
}

static int count_records(const char *msg, char *key, int in_tx) {
	test_req_t req, res;
	product_t *p = products;
	int cnt = -1;

	(void) strcpy(req.db, DBF);
	(void) strcpy(req.data, key);
	req.op = '1';

	if (in_tx || start_tx(TX_TYPE_BEGIN) == 0) {
		for (p = products; p->id != -1; p++) {
			req.prod = p->id;
			if (p->id != 0) {
				int rc = (p->access(&req, &res) == 0 ? atoi(res.data) : -1);

				if (rc == -1) {
					userlogc("Error: Db %d access error", p->id);
					return -1;
				}
				if (rc != cnt && cnt != -1) {
					userlogc("All databases should have the same no of records: db %d cnt %d (prev was %d)", p->id, rc, cnt);
					return -1;
				}

				cnt = rc;
			}
		}

		if (in_tx || end_tx(TX_TYPE_COMMIT) == 0)
			return cnt;
	}

	return cnt;
}

static int check_count(const char *msg, char *key, int in_tx, int expect) {
	int rcnt;

	if ((rcnt = count_records(msg, key, 0)) != expect) {
		userlogc("WRONG NUMBER OF RECORDS: %d expected %d", rcnt, expect);
		return -1;
	}

	return 0;
}

static int db_op(const char * msg, const char *dbfile, const char *data, char op, enum TX_TYPE txtype, char **prbuf, int remote) {
	if (msg)
		userlogc((char *) msg);

	if (start_tx(txtype) == 0) {
		int rv = 0;
		test_req_t req, res;
		product_t *p = products;

		init_req(&req, 0, dbfile, data, op, TX_TYPE_NONE);

		for (p = products; p->id != -1; p++) {
			req.prod = p->id;
			userlogc("invoke prod %d remote=%d dbf=%s", p->id, remote, dbfile);
			rv = (remote ? send_req(p->id, msg, dbfile, data, op, prbuf) : p->access(&req, &res));

			if (rv)
				userlogc("BAD REQ %d", rv);
		}

		if (end_tx(txtype) == 0)
			return rv;
	}

	return -1;
}

static int setup() {
	int rv;

	/* start off with no records */
	if ((rv = db_op("DELETE AT SETUP", DBF, emps[0], '3', TX_TYPE_BEGIN_COMMIT, 0, 0)) != 0)
		return rv;

	if ((rv = check_count("COUNT RECORDS", emps[0], 0, 0)))
		return rv;

	return 0;
}

static int teardown(int *cnt)
{
	int rv;

	/* delete records starting from emps[0], */
	if ((rv = db_op("LOCAL DELETE", DBF, emps[0], '3', TX_TYPE_BEGIN_COMMIT, 0, 0)))
		return rv;

	*cnt = 0;
	if ((rv = check_count("COUNT RECORDS", emps[0], 0, *cnt)))
		return rv;

	return 0;
}

static int test1(int *cnt)
{
	int rv;

	/* ask the remote service to insert a record */
	if ((rv = db_op("REMOTE INSERT 1", DBF, emps[5], '0', TX_TYPE_BEGIN, 0, 1)))
		return rv;

	/* ask the remote service to insert another record in the same transaction */
	if ((rv = db_op("REMOTE INSERT 2", DBF, emps[6], '0', TX_TYPE_NONE, 0, 1)))
		return rv;

	/* insert a record and end the already running transaction */
	if ((rv = db_op("LOCAL INSERT", DBF, emps[7], '0', TX_TYPE_COMMIT, 0, 0)))
		return rv;

	*cnt += 3;
	/* make sure the record count increases by 3 */
	if ((rv = check_count("COUNT RECORDS", emps[0], 0, *cnt)))
		return -1;

	return 0;
}

static int test2(int *cnt)
{
	int rv;

	/* ask the remote service to insert a record */
	if ((rv = db_op("REMOTE INSERT 1", DBF, emps[0], '0', TX_TYPE_BEGIN, 0, 1)))
		return rv;

	/* ask the remote service to insert another record in the same transaction */
	if ((rv = db_op("REMOTE INSERT 2", DBF, emps[1], '0', TX_TYPE_NONE, 0, 1)))
		return rv;

	/* insert a record and end the already running transaction */
	if ((rv = db_op("REMOTE INSERT 3", DBF, emps[2], '0', TX_TYPE_COMMIT, 0, 1)))
		return rv;

	*cnt += 3;
	/* make sure the record count increases by 3 */
	if ((rv = check_count("COUNT RECORDS", emps[0], 0, *cnt)))
		return -1;

	return 0;
}

static int test3(int *cnt)
{
	int rv;

	/* ask the remote service to insert a record */
	if ((rv = db_op("REMOTE INSERT 1", DBF, emps[3], '0', TX_TYPE_BEGIN_COMMIT, 0, 1)))
		return rv;

	*cnt += 1;
	/* delete records starting from emps[0] but abort it */
	if ((rv = db_op("REMOTE INSERT WITH ABORT", DBF, emps[4], '1', TX_TYPE_BEGIN_ABORT, 0, 1)))
		return rv;

	if ((rv = check_count("COUNT RECORDS", emps[0], 0, *cnt)))
		return rv;

	return 0;
}

static int test4(int *cnt)
{
	int rv;

	/* ask the remote service to insert a record */
	if ((rv = db_op("REMOTE INSERT 1", DBF, emps[4], '0', TX_TYPE_BEGIN_COMMIT, 0, 1)))
		return rv;

	*cnt += 1;
	/* modify one of the records */
	if ((rv = db_op("REMOTE UPDATE", DBF, emps[4], '2', TX_TYPE_BEGIN_COMMIT, 0, 1)))
		return rv;

	/* remote delete records starting from emps[0] with abort*/
	if ((rv = db_op("REMOTE DELETE WITH ABORT", DBF, emps[0], '3', TX_TYPE_BEGIN_ABORT, 0, 1)))
		return rv;

	/* local delete records starting from emps[0] with abort*/
	if ((rv = db_op("LOCAL DELETE WITH ABORT", DBF, emps[0], '3', TX_TYPE_BEGIN_ABORT, 0, 0)))
		return rv;

	if ((rv = check_count("COUNT RECORDS", emps[0], 0, *cnt)))
		return rv;

	return 0;
}

static int run_test()
{
	int rv, cnt = 0;

	if ((rv = setup()))
		return rv;

#if 0
	// TODO BDB hangs with this test
	if ((rv = test1(&cnt)))
		return rv;
#else
	if ((rv = test2(&cnt)))
		return rv;
	if ((rv = test3(&cnt)))
		return rv;
	if ((rv = test4(&cnt)))
		return rv;
#endif

	if ((rv = teardown(&cnt)))
		return rv;

	return 0;
}

int main(int argc, char **argv)
{
	int rv = -1;

DISABLE_XA = 0;
	if (argc <= 1 || strcmp("bdb", argv[1]) == 0)
		prod = BDB;
	else if (strcmp("ora", argv[1]) == 0)
		prod = ORACLE;
	else
		fatal("Unsupported db - use ora or bdb\n");

	if (tx_open() != TX_OK)
		fatal("ERROR - Could not open transaction: ");

	rv = run_test();

	if (tx_close() != TX_OK) {
		userlogc((char*) "ERROR - Could not close transaction: ");
		rv = -1;
	}

	fprintf(stdout, "Test %s (%d)\n", (rv ? "failed" : "passed"), rv);

	return rv;
}
