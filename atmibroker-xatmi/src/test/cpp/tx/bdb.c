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
#include <tx.h>

#include "tx/request.h"

#ifdef BDB
#include <bdb/db.h> /* Berkeley dB include */

/* BerkeleyDb X/Open Resource Manager entry points */
#ifdef WIN32
extern __declspec(dllimport) struct xa_switch_t db_xa_switch;
#define snprintf _snprintf
#else
struct xa_switch_t db_xa_switch;
#endif

static void init_DBT(DBT *val, char *v)
{
	memset(val, 0, sizeof(DBT));
	val->data = v;
	if (v) val->size = strlen(v);
}

static void init_rec(DBT *key, DBT *val, char *k, char *v)
{
	if (key) init_DBT(key, k);
	if (val) init_DBT(val, v);
}

/**
 * acquire a cursor for the database.
 */
static DBC * open_cursor(DB *dbp, int *ret)
{
	DBC *dbcp = NULL;

	if ((*ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0)
		dbp->err(dbp, *ret, "DB->cursor");

	return dbcp;
}

static int dexists(const char * fname)
{
	FILE *f = fopen(fname, "r");

	if (!f) {
		return 0;
	} else {
		fclose(f);
	}

	return 1;
}

/*
 * Open a Berkeley database whose backing file and database names
 * correspond to the input parameters.
 */
static int opendb(DB **dbp, const char *backing_file, const char * dbname)
{
	int ret;

	if ((ret = db_create(dbp, NULL, DB_XA_CREATE)) != 0 && !dexists(backing_file)) {
		logit(0, (char*) "db_create error %d (%s)", ret, backing_file);
		(*dbp)->err(*dbp, ret, "DB->create %d", ret);
	} else if ((ret = (*dbp)->open(*dbp, NULL, backing_file, dbname, DB_BTREE, DB_CREATE, 0664)) != 0) {
		logit(0, (char*) "db_open error %d (%s)", ret, backing_file);
		(*dbp)->err(*dbp, ret, "DB->open %d", ret);
	} else {
		logit(1, (char*) "db_create and db_open of %s ok", backing_file);
	}

	return ret;
}

static int doInsert(DB *dbp, char *k, char *v) {
	DBT key, data;
	int ret;

	init_rec(&key, &data, k, v);

	logit(1, (char*) "insert %s=%s", k, v);

	if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) != 0)
		dbp->err(dbp, ret, "DB->put");

	return ret;
}

static int doUpdate(DB *dbp, char *k, char *v) {
	DBT key, data;
	int ret;

	init_rec(&key, &data, k, v);

	logit(1, (char*) "update %s=%s", k, v);

	if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) != 0)
		dbp->err(dbp, ret, "DB->update");

	return ret;
}

static int doDelete(DB *dbp, char *k) {
	DBT key, data;
	int ret = 0;

	k = 0;
   	init_rec(&key, &data, k, 0);

	logit(1, (char*) "delete %s", k);

	if (k != 0 && strlen(k) != 0) {
		if ((ret = dbp->del(dbp, NULL, &key, 0)) != 0)
			dbp->err(dbp, ret, "DB->del");
	} else if (k == NULL || strlen(k) == 0) {
		DBC *dbcp = open_cursor(dbp, &ret);

		if (ret != 0)
			return ret;

		while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
			if ((ret = dbp->del(dbp, NULL, &key, 0)) != 0)
				dbp->err(dbp, ret, "DB->del");
		}

		return 0;
	}

	return ret;
}

static int doSelect(DB *dbp, char *kv, int *rcnt) {
	int ret;
	DBT key, data;
	DBC *dbcp;
	char **vv = 0;

	dbcp = open_cursor(dbp, &ret);
	logit(1, "doSelect key=%s", kv);
	*rcnt = 0;

	if (ret != 0)
		return ret;

	init_rec(&key, &data, 0, 0);

	if (vv != NULL)
		**vv = 0;

	while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0) {
		logit(1, "record: %s=%s", (char *) key.data, (char *) data.data);
		*rcnt += 1;
		if (vv != NULL && kv != NULL && strcmp(kv, (char *) key.data) == 0) {
			strcpy(*vv, (char *) data.data);
			*(*vv + data.size) = 0;
		}
	}

	logit(1, "doSelect %d records", *rcnt);
	if (ret != DB_NOTFOUND)
		dbp->err(dbp, ret, "DBcursor->get");

	if ((ret = dbcp->c_close(dbcp)) != 0)
		dbp->err(dbp, ret, "cursor->c_close");

	return ret;
}

static int doWork(DB *dbp, char op, char *arg, test_req_t *resp)
{
	int status = 0;
	char *v1 = "Jim,Janitor,7902,17-DEC-80,900,0,20";
	char *v2 = "Jim,Director,7902,17-DEC-80,900,0,20";

	logit(1, "doWork op=%c arg=%s", op, arg);
	(resp->data)[0] = 0;

	if (op == '0') {
		status = doInsert(dbp, arg, v1);
	} else if (op == '1') {
		int rcnt = 0;   // no of matching records
		status = doSelect(dbp, arg, &rcnt);
		snprintf(resp->data, sizeof (resp->data), "%d", rcnt);
	} else if (op == '2') {
		status = doUpdate(dbp, arg, v2);
	} else if (op == '3') {
		status = doDelete(dbp, arg);
	}

	return status;
}

long bdb_xaflags()
{
	return db_xa_switch.flags;
}

int bdb_access(test_req_t *req, test_req_t *resp)
{
	DB *dbp;
	int stat, rv;

	logit(1, (char*) "bdb_access");
	if ((stat = opendb(&dbp, req->db, NULL)) != 0)
		return fail("db open error", stat);

	logit(1, (char*) "bdb_access doWork");
	if ((rv = doWork(dbp, req->op, req->data, resp)) != 0)
		dbp->err(dbp, rv, "doWork");

	if ((stat = dbp->close(dbp, 0)) != 0)
		return fail("db close error", stat);

	return rv;
}
#endif
