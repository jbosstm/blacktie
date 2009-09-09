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
#ifndef __REQUEST_H
#define __REQUEST_H

#include "atmiBrokerCoreMacro.h"
#include "userlogc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <userlogc.h>
#include <xatmi.h>
#include <tx.h>

#ifdef UNITTEST
#define BDB
static const char * const TXTEST_SVC_NAME = "tpcall_x_octet";
#else
static const char * const TXTEST_SVC_NAME = "BAR";
#endif

#define TX_TYPE_NONE	0x0
#define TX_TYPE_BEGIN	0x1
#define TX_TYPE_COMMIT	0x2
#define TX_TYPE_ABORT	0x4
#define TX_TYPE_HALT	0x8

#define TX_TYPE_END				(TX_TYPE_COMMIT | TX_TYPE_ABORT)
#define TX_TYPE_BEGIN_COMMIT	(TX_TYPE_BEGIN | TX_TYPE_COMMIT)
#define TX_TYPE_BEGIN_ABORT		(TX_TYPE_ABORT | TX_TYPE_COMMIT)

typedef struct BLACKTIE_XATMI_DLL test_req {
	char db[16];
	char data[80];
	char op;
	int  id;	// request id
	int  expect;	// for testing null products (ie force success)
	int  prod;
	int txtype;
	int status;
} test_req_t;

#ifndef ORACLE
#define ora_access	null_access
#define ora_xaflags	null_xaflags
#endif

#ifndef BDB
#define bdb_access	null_access
#define bdb_xaflags	null_xaflags
#endif

/*
 * some RMs do not allow mixed access - in fact Berkeley Db doesn't even support
 * 2 dbs in different files one accessed remotely and the other locally
 */

#define REMOTE_ACCESS   0x1
#define LOCAL_ACCESS	0x2
#define ANY_ACCESS  (REMOTE_ACCESS | LOCAL_ACCESS)

typedef struct BLACKTIE_XATMI_DLL product {
	int id; /* id for the product (used by servers to index into products) */
	const char *pname;  /* a name for the product configuration */
	const char *dbname; /* identifies the database */
	int loc;	/* bitmap indicating what kind of access is allowed */
	int (*access)(test_req_t *, test_req_t *);  // function for doing CRUD operations
	long (*xaflags)();  // function to return the flags supported by the Resource Manager
} product_t;

extern product_t products[];

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
extern __declspec(dllimport) struct xa_switch_t db_xa_switch;
#define snprintf _snprintf
#endif

/* common methods */
extern int fatal(const char *msg);
extern int fail(const char *reason, int ret);
extern test_req_t * get_buf(int remote, const char *data, const char *dbfile, char op, int prod, int txtype, int expect);
extern void free_buf(int remote, test_req_t *req);

/* helper methods for controling transactions */

extern  int is_begin(int txtype);
extern int is_commit(int txtype);
extern int is_abort(int txtype);
extern int start_tx(int txtype);
extern int end_tx(int txtype);

extern int null_access(test_req_t *req, test_req_t *resp);
extern int ora_access(test_req_t *req, test_req_t *resp);
extern int bdb_access(test_req_t *req, test_req_t *resp);
extern int is_tx_in_state(int txtype);

extern long null_xaflags();
extern long ora_xaflags();
extern long bdb_xaflags();

#ifdef __cplusplus
}
#endif

#endif /* __REQUEST_H */
