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

#include <xatmi.h>
#include <tx.h>

#include "products.h"

static product_t prods[8];
static char testid[16];
static char *emps[] = {"8000", "8001", "8002", "8003", "8004", "8005", "8006", "8007"};

static product_t *get_product(const char *pid) {
    int id = atoi((char *) pid);
    product_t *p;

    for (p = products; p->id != -1; p++)
        if (p->id == id)
            return p;

    return 0;
}

static void set_test_id(const char *id) {
	(void) strncpy(testid, id, sizeof (testid));
}

static int send_req(test_req_t *req, char **prbuf) {
	long rsz = sizeof (test_req_t);
	long callflags = 0L;
	test_req_t *resp;
	int rv = 0;

	resp = (test_req_t *) tpalloc((char*) "X_C_TYPE", (char*) "dc_buf", sizeof (test_req_t));

	logit(0, (char*) "Invoke Service DBS %4d: prod=%d op=%c data=%s dbf=%s", req->id, req->prod, req->op, req->data, req->db);
	if (tpcall("DBS", (char *) req, sizeof (test_req_t), (char **) &resp, &rsz, callflags) == -1) {
		logit(0, (char*) "TP ERROR tperrno: %d", tperrno);
		rv = -1;
	} else if (prbuf && *prbuf) {
		strncpy(*prbuf, resp->data, sizeof (resp->data));
	}

	tpfree((char *) resp);

	return rv;
}

static int count_records(const char *msg, char *key, int in_tx, int expect) {
	int cnt = -1;

	if (in_tx || start_tx(TX_TYPE_BEGIN) == 0) {
		int rv = 0;
		test_req_t *req;
		test_req_t res;
		product_t *p = prods;
		char *rbuf = (char *) (res.data);

		for (p = prods; p->id != -1; p++) {
			int remote = LOCAL_ACCESS;

			if ((p->loc & remote) == 0)	/* the RM does not support the requested access type */
				remote = p->loc;

			req = get_buf((remote & REMOTE_ACCESS), key, p->dbname, '1', p->id, TX_TYPE_NONE, expect);
			logit(0, "invoke prod %s (id=%d) remote=%d dbf=%s", p->pname, p->id, remote, p->dbname);

			rv = ((remote & REMOTE_ACCESS) ? send_req(req, &rbuf) : p->access(req, &res));
logit(0, "invoked ok");

			if (rv)
				logit(0, "BAD REQ %d", rv);

			free_buf(remote, req);
			rv = (rv == 0 ? atoi(res.data) : -1);

logit(0, "and count is %d", rv);
			if (rv == -1) {
				logit(0, "Error: Db %d access error", p->id);
				return -1;
			}
			if (rv != cnt && cnt != -1) {
				logit(0, "All databases should have the same no of records: db %d cnt %d (prev was %d)", p->id, rv, cnt);
				return -1;
			}

			cnt = rv;
		}

		if (in_tx || end_tx(TX_TYPE_COMMIT) == 0)
			return cnt;
	}

	return cnt;
}

static int check_count(const char *msg, char *key, int in_tx, int expect) {
	int rcnt;

	if ((rcnt = count_records(msg, key, 0, expect)) != expect) {
		logit(0, "WRONG NUMBER OF RECORDS: %d expected %d", rcnt, expect);
		return -1;
	}

	logit(0, "%s: RECORD COUNT: %d expected %d", testid, rcnt, expect);
	return 0;
}

static int db_op(const char *msg, const char *data, char op, enum TX_TYPE txtype,
				 char **prbuf, int remote, int migrating, int expect) {
	if (msg)
		logit(0, (char *) "%s: %s %s", testid, ((remote | REMOTE_ACCESS) ? "REMOTE" : "LOCAL"), msg);

	if (start_tx(txtype) == 0) {
		int rv = 0;
		test_req_t *req;
		test_req_t res;
		product_t *p = prods;

		for (p = prods; p->id != -1; p++) {
#if 0
			if (migrating && (p->xaflags() & TMNOMIGRATE)) {
				/* the RM does not support tx migration (see XA spec for explanation */
				logit(0, "Info: RM %d does not support tx migration (switching from remote)", p->id);
				remote = !remote;
			}
#endif

			if ((p->loc & remote) == 0)	/* the RM does not support the requested access type */
				remote = p->loc;

			req = get_buf((remote & REMOTE_ACCESS), data, p->dbname, op, p->id, TX_TYPE_NONE, expect);
			logit(0, "invoke prod %s (id=%d) remote=%d dbf=%s", p->pname, p->id, remote, p->dbname);

			rv = ((remote & REMOTE_ACCESS) ? send_req(req, prbuf) : p->access(req, &res));

			if (rv)
				logit(0, "BAD REQ %d", rv);

			free_buf(remote, req);
		}

		if (end_tx(txtype) == 0)
			return rv;
	}

	return -1;
}

/**
 * ensure that all the target databases start off in the same state
 */
static int setup() {
	int rv;

	/* start off with no records */
	if ((rv = db_op("DELETE AT SETUP", emps[0], '3', TX_TYPE_BEGIN_COMMIT, 0, LOCAL_ACCESS, 0, -1)) != 0)
		return rv;

	if ((rv = check_count("COUNT RECORDS", emps[0], 0, 0)))
		return rv;

	return 0;
}

/**
 * remove all the records added by the test
 */
static int teardown(int *cnt)
{
	int rv;

	/* delete records starting from emps[0], */
	if ((rv = db_op("DELETE", emps[0], '3', TX_TYPE_BEGIN_COMMIT, 0, LOCAL_ACCESS, 0, -1)))
		return rv;

	*cnt = 0;
	if ((rv = check_count("COUNT RECORDS", emps[0], 0, *cnt)))
		return rv;

	return 0;
}

static int test0(int *cnt)
{
	int rv;

	set_test_id("Test 0");
#if 1
	/* ask the remote service to insert a record */
	if ((rv = db_op("INSERT 1", emps[5], '0', TX_TYPE_BEGIN, 0, REMOTE_ACCESS, 0, -1)))
		return rv;

	/* ask the remote service to insert another record in the same transaction */
	if ((rv = db_op("INSERT 2", emps[6], '0', TX_TYPE_NONE, 0, REMOTE_ACCESS, 0, -1)))
		return rv;

	/* insert a record and end the already running transaction */
	if ((rv = db_op("INSERT", emps[7], '0', TX_TYPE_COMMIT, 0, LOCAL_ACCESS, 1, -1)))
		return rv;

	*cnt += 3;
#else
	if ((rv = db_op("INSERT 1", emps[5], '0', TX_TYPE_BEGIN_COMMIT, 0, LOCAL_ACCESS, 0, -1)))
		return rv;
	*cnt += 1;
#endif
	/* make sure the record count increases by 3 */
	if ((rv = check_count("COUNT RECORDS", emps[0], 0, *cnt)))
		return -1;

	return 0;
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
static int test1(int *cnt)
{
	int rv;

	set_test_id("Test 1");
	/* ask the remote service to insert a record */
	if ((rv = db_op("INSERT 1", emps[5], '0', TX_TYPE_BEGIN, 0, REMOTE_ACCESS, 0, -1)))
		return rv;

	/* ask the remote service to insert another record in the same transaction */
	if ((rv = db_op("INSERT 2", emps[6], '0', TX_TYPE_NONE, 0, REMOTE_ACCESS, 0, -1)))
		return rv;
	/* insert a record and end the already running transaction */
	if ((rv = db_op("INSERT", emps[7], '0', TX_TYPE_COMMIT, 0, LOCAL_ACCESS, 1, -1)))
		return rv;

	*cnt += 3;

	/* make sure the record count increases by 3 */
	if ((rv = check_count("COUNT RECORDS", emps[0], 0, *cnt)))
		return -1;

	return 0;
}

/**
 * start a transaction
 * perform a remote insert on each target db
 * perform another remote insert on each target db
 * perform a third remote insert on each target db
 * commit the transaction
 */
static int test2(int *cnt)
{
	int rv;

	set_test_id("Test 2");
	/* ask the remote service to insert a record */
	if ((rv = db_op("INSERT 1", emps[0], '0', TX_TYPE_BEGIN, 0, REMOTE_ACCESS, 0, -1)))
		return rv;

	/* ask the remote service to insert another record in the same transaction */
	if ((rv = db_op("INSERT 2", emps[1], '0', TX_TYPE_NONE, 0, REMOTE_ACCESS, 0, -1)))
		return rv;

	/* insert a record and end the already running transaction */
	if ((rv = db_op("INSERT 3", emps[2], '0', TX_TYPE_COMMIT, 0, REMOTE_ACCESS, 0, -1)))
		return rv;

	*cnt += 3;
	/* make sure the record count increases by 3 */
	if ((rv = check_count("COUNT RECORDS", emps[0], 0, *cnt)))
		return -1;

	return 0;
}

/**
 * start a transaction
 * insert a record into each db
 * commit the transaction
 *
 * repeat the test but rollback the transaction instead of commit
 */
static int test3(int *cnt)
{
	int rv;

	set_test_id("Test 3");
	/* ask the remote service to insert a record */
	if ((rv = db_op("INSERT 1", emps[3], '0', TX_TYPE_BEGIN_COMMIT, 0, REMOTE_ACCESS, 0, -1)))
		return rv;

	*cnt += 1;
	/* delete records starting from emps[0] but abort it */
	if ((rv = db_op("INSERT WITH ABORT", emps[4], '1', TX_TYPE_BEGIN_ABORT, 0, REMOTE_ACCESS, 0, -1)))
		return rv;

	if ((rv = check_count("COUNT RECORDS", emps[0], 0, *cnt)))
		return rv;

	return 0;
}

/**
 * start tx, do a remote insert on all dbs commit the tx
 * start tx, do a remote update on all dbs commit the tx
 * start tx, do a remote delete on all dbs abort the tx
 * start tx, do a local delete on all dbs abort the tx
 */
static int test4(int *cnt)
{
	int rv;

	set_test_id("Test 4");
	/* ask the remote service to insert a record */
	if ((rv = db_op("INSERT 1", emps[4], '0', TX_TYPE_BEGIN_COMMIT, 0, REMOTE_ACCESS, 0, -1)))
		return rv;

	*cnt += 1;
	/* modify one of the records */
	if ((rv = db_op("UPDATE", emps[4], '2', TX_TYPE_BEGIN_COMMIT, 0, REMOTE_ACCESS, 0, -1)))
		return rv;

	/* remote delete records starting from emps[0] with abort*/
	if ((rv = db_op("DELETE WITH ABORT", emps[0], '3', TX_TYPE_BEGIN_ABORT, 0, REMOTE_ACCESS, 0, -1)))
		return rv;

	/* local delete records starting from emps[0] with abort*/
	if ((rv = db_op("DELETE WITH ABORT", emps[0], '3', TX_TYPE_BEGIN_ABORT, 0, LOCAL_ACCESS, 0, -1)))
		return rv;

	if ((rv = check_count("COUNT RECORDS", emps[0], 0, *cnt)))
		return rv;

	return 0;
}

static int run_tests()
{
	int rv, i, cnt = 0;
	struct test {
		int (*test)(int *);
	} tests[] = {
#if 0
		{test0},
#else
		{test1},
		{test2},
		{test3},
		{test4},
#endif
		{0}
	};

	if ((rv = setup()))
		return rv;

    for (i = 0; tests[i].test != 0; i++)
		if ((rv = tests[i].test(&cnt)))
			return rv;
	if ((rv = teardown(&cnt)))
		return rv;

	return 0;
}

int main(int argc, char **argv)
{
	int rv = -1;
	int i;

#if 0
logit(0, (char*) "open");
rv = tx_open();
logit(0, (char*) "open rv=%d", rv);
rv = tx_close();
logit(0, (char*) "close rv=%d", rv);
return 0;
#endif
    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            product_t *p = get_product(argv[i]);

            if (p == NULL)
				fatal("Requested db is not supported\n");

            prods[i - 1] = *p;
        }

        prods[i - 1].id = -1;
        prods[i - 1].access = 0;
        prods[i - 1].xaflags = 0;
    } else {
        for (i = 0; products[i].id != -1; i++)
            prods[i] = products[i];

		prods[i] = products[i];
    }

	if (tx_open() != TX_OK)
		fatal("ERROR - Could not open transaction: ");

	for (i = 0; prods[i].id != -1; i++)
		logit(0, (char*) "INFO: %s (%s) id=%d flags=0x%x", prods[i].pname, prods[i].dbname, prods[i].id, prods[i].xaflags());

	rv = run_tests();

	if (tx_close() != TX_OK) {
		logit(0, (char*) "ERROR - Could not close transaction: ");
		rv = -1;
	}

	fprintf(stdout, "Test %s (%d)\n", (rv ? "failed" : "passed"), rv);

	return rv;
}
