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

#undef ORACLE
#define BDB

#ifndef ORACLE
#define ora_access	null_access
#define ora_xaflags	null_xaflags
#endif

#ifndef BDB
#define bdb_access	null_access
#define bdb_xaflags	null_xaflags
#endif

enum TX_TYPE {
	TX_TYPE_BEGIN,
	TX_TYPE_COMMIT,
	TX_TYPE_ABORT,
	TX_TYPE_BEGIN_COMMIT,
	TX_TYPE_BEGIN_ABORT,
	TX_TYPE_NONE,
};

typedef struct test_req {
	char db[16];
	char data[80];
	char op;
	int  id;	// request id
	int  expect;	// for testing null products (ie force success)
	int  prod;
	enum TX_TYPE txtype;
	int status;
} test_req_t;

/* common methods */
void fatal(const char *msg);
int fail(const char *reason, int ret);
void logit(int debug, const char * format, ...);
test_req_t * get_buf(int remote, const char *data, const char *dbfile, char op, int prod, enum TX_TYPE txtype, int expect);
void free_buf(int remote, test_req_t *req);

/* helper methods for controling transactions */
int is_begin(enum TX_TYPE txtype);
int is_commit(enum TX_TYPE txtype);
int is_abort(enum TX_TYPE txtype);
int start_tx(enum TX_TYPE txtype);
int end_tx(enum TX_TYPE txtype);

#endif /* __REQUEST_H */
