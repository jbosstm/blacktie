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
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <db.h>	/* Berkeley dB include */

#define BLACKTIE	/* use Blacktie as the transaction manager */

#ifdef BLACKTIE
#include "txClient.h"	/* Blacktie include */
#endif

extern struct xa_switch_t db_xa_switch;	/* the X/Open Resource Manager entry points */

#ifndef BLACKTIE
static XID xids[2];
static int rmids[2] = {101, 102};
static int xid_cnt = 2;

/* generate an arbitary but unique XID */
static long nextTm = 0L;
static void new_xid(XID *xid)
{
	int i ;

	nextTm = (nextTm == 0L ? (long) time(NULL) : nextTm + 1);

	memset(xid, 0, sizeof(xid));
	xid->formatID = 1L;

	xid->gtrid_length = 4;
	for (i = 0; i < 4; i++)
		xid->data[i] = i;

	xid->bqual_length = 4;
	for (i = 4; i < 8; i++)
		xid->data[i] = (nextTm << ((i - 4) * 8));
}

static int xaop(const char *op, int rmid, XID *xid) {
        int ret = XA_OK;

        if (strcmp(op, "open") == 0)
                ret = (db_xa_switch.xa_open_entry)((char *) "." , rmid, TMNOFLAGS);
        else if (strcmp(op, "close") == 0)
                ret = (db_xa_switch.xa_close_entry)((char *) "" , rmid, TMNOFLAGS);
        else if (strcmp(op, "start") == 0)
                ret = (db_xa_switch.xa_start_entry)(xid, rmid, TMNOFLAGS);
        else if (strcmp(op, "end") == 0)
                ret = (db_xa_switch.xa_end_entry)(xid, rmid, TMSUCCESS);
        else if (strcmp(op, "close") == 0)
                ret = (db_xa_switch.xa_close_entry)((char *) "", rmid, TMNOFLAGS);
        else if (strcmp(op, "prepare") == 0)
                ret = (db_xa_switch.xa_prepare_entry)(xid, rmid, TMNOFLAGS);
        else if (strcmp(op, "commit") == 0)
                ret = (db_xa_switch.xa_commit_entry)(xid, rmid, TMNOFLAGS);
        else if (strcmp(op, "rollback") == 0)
                ret = (db_xa_switch.xa_rollback_entry)(xid, rmid, TMNOFLAGS);

        if (ret != XA_OK)
                fprintf(stderr, "%s error: %d\n", op, ret);

        return ret;
}
#endif

int fail(const char *reason, int ret)
{
	fprintf(stderr, "%s: %d\n", reason, ret);
	return ret;
}

void init_DBT(DBT *val, char *v)
{
	memset(val, 0, sizeof(DBT));
	val->data = v;
	if (v) val->size = strlen(v);
}

void init_rec(DBT *key, DBT *val, char *k, char *v)
{
	if (key) init_DBT(key, k);
	if (val) init_DBT(val, v);
}

/**
 * insert a record
 */
int put_rec(DB *dbp, char *k, char *v)
{
	DBT key, data;
	int ret;

	init_rec(&key, &data, k, v);

	if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) != 0)
		dbp->err(dbp, ret, "DB->put");

	return ret;
}

/**
 * delete the record whose key is k
 */
int del_rec(DB *dbp, char *k)
{
	DBT key;
	int ret;

	init_DBT(&key, k);

	if ((ret = dbp->del(dbp, NULL, &key, 0)) != 0)
		dbp->err(dbp, ret, "DB->put");

	return ret;
}

/**
 * acquire a cursor for the database.
 */
DBC * open_cursor(DB *dbp, int *ret)
{
	DBC *dbcp = NULL;

	if ((*ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0)
		dbp->err(dbp, *ret, "DB->cursor");

	return dbcp;
}

/**
 * Print records on standard output. The input parameter
 * corresponds to a record key - if it is non NULL then
 * only that record is printed, otherwise all records
 * are printed.
 */
int dumpdb(DB *dbp, char *kv)
{
	int ret;
	DBT key, data;
	DBC *dbcp = open_cursor(dbp, &ret);

	if (ret != 0) {
		dbp->err(dbp, ret, "DB->cursor");
		return ret;
	}

	init_rec(&key, &data, 0, 0);

	while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
		if (kv == NULL || strcmp(kv, (char *)key.data) == 0)
			printf("%.*s : %.*s\n",
				(int)key.size, (char *)key.data,
				(int)data.size, (char *)data.data);
	}

	if (ret != DB_NOTFOUND)
		dbp->err(dbp, ret, "DBcursor->get");

	if ((ret = dbcp->c_close(dbcp)) != 0)
		dbp->err(dbp, ret, "cursor->c_close");

	return ret;
}

int fexists(const char * fname)
{
	FILE *f = fopen(fname, "r");

	if (!f) return 0;

	fclose(f);

	return 1;
}

/**
 * Open a Berkeley database whose backing file and database names
 * correspond to the input parameters.
 */
int opendb(DB **dbp, const char *backing_file, const char * dbname)
{
	int ret;

	if ((ret = db_create(dbp, NULL, DB_XA_CREATE)) != 0 && !fexists(dbname))
		return fail("db_create error", ret);

        if ((ret = (*dbp)->open(*dbp, NULL, backing_file, dbname, DB_BTREE, DB_CREATE, 0664)) != 0)
		(*dbp)->err(*dbp, ret, "DB->open");

	return ret;
}

int closedb(DB *dbp)
{
	int ret;

	if ((ret = dbp->close(dbp, 0)) != 0)
		dbp->err(dbp, ret, "DB->close");

	return ret;
}

/**
 * Start a transaction. The tx_open call opens all Resource Managers
 * associated with the application, if BLACKTIE is defined. If
 * BLACKTIE is not defined then the Resource Managers identified
 * by in the variable rmids are explicitly opened.
 */
int startTx()
{
	int ret;
#ifdef BLACKTIE
	if ((ret = tx_open()) != TX_OK)
		return fail("tx open err", ret);

	if ((ret = tx_begin()) != TX_OK)
		return fail("tx begin err", ret);
#else
	int i;
	new_xid(&xids[0]);
	new_xid(&xids[1]);

	for (i = 0; i < xid_cnt; i++) {
		if ((ret = xaop("open", rmids[i], &xids[i])))
			return fail("xa open err", ret);
		if ((ret = xaop("start", rmids[i], &xids[i])))
			return fail("xa open err", ret);
	}
#endif
}

/**
 * if the input parameter is non zero then commit the current transaction,
 * otherwise roll it back
 */
int terminateTx(int commit)
{
	int ret;
#ifdef BLACKTIE
	ret = (commit ? tx_commit() : tx_rollback());
	(void) tx_close();

	if (ret != TX_OK)
		return fail("tx complete err", ret);
#else
	int i;

	for (i = 0; i < xid_cnt; i++)
		(void) xaop("end", rmids[i], &xids[i]);

	if (commit) {
		for (i = 0; i < xid_cnt; i++)
			(void) xaop("prepare", rmids[i], &xids[i]);
		for (i = 0; i < xid_cnt; i++)
			(void) xaop("commit", rmids[i], &xids[i]);
	} else {
		for (i = 0; i < xid_cnt; i++)
			(void) xaop("rollback", rmids[i], &xids[i]);
	}

	for (i = 0; i < xid_cnt; i++)
		(void) xaop("close", rmids[i], &xids[i]);
#endif
}

/**
 * read, write or delete records
 */
int updateDb(const char * backingfile, const char * dbname, int argc, char * argv[])
{
	int ret1 = 0, ret2 = 0, i;
	DB *dbp;
	char op = (argc > 1 ? *argv[1] : 'r');

	(void) opendb(&dbp, backingfile, dbname);

	/* do some work */
	if (op == 'r') {
		dumpdb(dbp, 0);
	} else if (op == 'd') {
		for (i = 2; i < argc; i+=2)
			ret1 = del_rec(dbp, argv[i]);
	} else if (op == 'w') {
		if (argc % 2 != 0)
			return fail("insuffient args", -1);

		for (i = 2; i < argc; i+=2)
			ret1 = put_rec(dbp, argv[i], argv[i + 1]);
	}

	if ((ret2 = dbp->close(dbp, 0)) != 0)
		return fail("db close error", ret2);

	return ret1;
}

static const char *dbfile1 = "db1";	/* backing file for database 1 */
static const char *dbfile2 = "db2";	/* backing file for database 2 */
static const char *db1name = "db1";	/* name of database 1 */
static const char *db2name = "db2";	/* name of database 2 */

/**
 * Transactionally insert a record into two different databases.
 * if the input parameter is non zero the transaction is committed
 * otherwise it is rolled back.
 *
 * Then try to delete the insert record. The test verifies that the delete
 * is successful if the transaction was committed and vica-versa if it
 * was rolled back.
 */
int test1(int commit)
{
	int ret1 = 0, ret2 = 0;
	char * args1[] = {"xatest", "w", "1", "Adam"};
	char * args2[] = {"xatest", "d", "1"};

	/* insert a record into two different databases */
	printf("starting transaction 1\n");
        (void) startTx();
	(void) updateDb(dbfile1, 0, 4, args1);
	(void) updateDb(dbfile2, 0, 4, args1);
	printf("terminating transaction 1\n");
        (void) terminateTx(commit);

	/* remove the record that was just inserted (from both databases) */
	printf("starting transaction 2\n");
        (void) startTx();
	ret1 = updateDb(dbfile1, 0, 3, args2);
	ret2 = updateDb(dbfile2, 0, 3, args2);
	printf("terminating transaction 1\n");
        (void) terminateTx(commit);	/* commit or rollback the tx */

	/*
	 * - if the insert was committed then the attempt to delete the record should have succeeded
	 * - if the insert was rolled back then the attempt to delete the record should have returned DB_NOTFOUND
	 */
	printf("testing result\n");
	if ((commit && (ret1 != 0 || ret2 != 0)) ||
	    (!commit && (ret1 != DB_NOTFOUND && ret2 != DB_NOTFOUND))) {
		printf("FAIL: rvals: %d and %d\n", ret1, ret2);
		return -1;
	}

	printf("SUCCESS: rvals: %d and %d\n", ret1, ret2);
	return 0;
}

int main(int argc, char * argv[])
{
	(void) test1(1);	/* insert records and then commit */
	(void) test1(0);	/* insert records and then rollback */

	return 0;
}
