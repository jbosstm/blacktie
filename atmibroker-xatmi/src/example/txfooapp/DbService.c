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
#include <stdlib.h>
#include <string.h>

#include "xatmi.h"
#include "tx.h"
#include "userlogc.h"

#include "request.h"

#include <db.h> /* Berkeley dB include */

extern int ora_test();
extern struct xa_switch_t db_xa_switch; /* the X/Open Resource Manager entry points */

int fail(const char *reason, int ret)
{
    fprintf(stderr, "%s: %d\n", reason, ret);
    return ret;
}

/**
 * Start a transaction. The tx_open call opens all Resource Managers
 * associated with the application.
 */
int startTx()
{
    int ret;

    if ((ret = tx_open()) != TX_OK)
        return fail("tx open err", ret);

    if ((ret = tx_begin()) != TX_OK)
        return fail("tx begin err", ret);

	return 0;
}

/**
 * if the input parameter is non zero then commit the current transaction,
 * otherwise roll it back
 */
int terminateTx(int commit)
{
    int ret = (commit ? tx_commit() : tx_rollback());

    (void) tx_close();

    return (ret != TX_OK ? fail("tx complete err", ret) : TX_OK);
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

	userlogc((char*) "insert %s=%s", k, v);
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
        dbp->err(dbp, ret, "DB->del");

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
int dumpdb(DB *dbp, char *kv, char **vv, int clean)

{
    int ret;
    DBT key, data;
    DBC *dbcp = open_cursor(dbp, &ret);

    if (ret != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        return ret;
    }

    init_rec(&key, &data, 0, 0);

	if (vv != NULL)
		**vv = 0;

	if (clean) {
		while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
			if ((ret = dbp->del(dbp, NULL, &key, 0)) != 0)
				dbp->err(dbp, ret, "DB->del");
		}

		return 0;
	}

    while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
		if (vv != NULL && kv != NULL && strcmp(kv, (char *)key.data) == 0) {
			strcpy(*vv, (char *)data.data);
			*(*vv + data.size) = 0;
		}
#if 0
        if (kv == NULL || strcmp(kv, (char *)key.data) == 0)
            printf("%.*s : %.*s\n",
                (int)key.size, (char *)key.data,
                (int)data.size, (char *)data.data);
#endif
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

/*
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
 * read, write or delete records
 */
static int updateDb(const char * backingfile, const char * dbname, char ** rdata, int argc, char * argv[])
{
    int ret1 = 0, ret2 = 0, i;
    DB *dbp;
    char op = (argc > 1 ? *argv[1] : 'r');

    (void) opendb(&dbp, backingfile, dbname);

    /* do some work */
    if (op == 'r') {
        dumpdb(dbp, argv[2], rdata, 0);
    } else if (op == 'd') {
		if (argv[2] == NULL)
        	dumpdb(dbp, NULL, rdata, 1);

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

void parse_request(char *data, int len, char **rdata)
{
    char *key, *val;
	char *args[] = {"xatest", "", "", ""};
	test_req_t *req = (test_req_t *) data;

    key = strtok(req->data, "=");
	val = (key == 0 ? 0 : strtok(NULL, "="));

	args[1] = (val ? "w" : "r");
	args[2] = key;
	args[3] = val;

	if (req->op == 'd' || key == 0)
		args[1] = "d";

	(void) updateDb(req->db, 0, rdata, 4, args);

	userlogc((char*) "db: %s a1=%s a2=%s a3=%s (rbuf: %s)", req->db, args[1], args[2], args[3], (rdata != 0 ? *rdata : "null"));
}

void oracle_test(char *data, int len, char **rdata, long sz)
{
	test_req_t *req = (test_req_t *) data;

	userlogc((char*) "db=%s data: %s op=%d tx=%d", req->db, req->data, req->op, req->txtype);
	(void) ora_test((int) req->op, req->data, rdata, sz);
}

void DBS(TPSVCINFO * svcinfo) {
	char* buffer;
	int sendlen;

/*	userlogc((char*) "dbs called  - svc: %s data %s len: %d flags: %d",
 		 svcinfo->name, svcinfo->data, svcinfo->len, svcinfo->flags);*/

	sendlen = XATMI_SERVICE_NAME_LENGTH + 11;
	buffer = tpalloc("X_OCTET", 0, sizeof(char) * sendlen);
	*buffer = 0;
#if 0
	parse_request(svcinfo->data, svcinfo->len, &buffer);
#else
	oracle_test(svcinfo->data, svcinfo->len, &buffer, sendlen);
#endif

/*	userlogc((char*) "returning %s", buffer);*/
	tpreturn(1, 1, buffer, sendlen, 0);
}
