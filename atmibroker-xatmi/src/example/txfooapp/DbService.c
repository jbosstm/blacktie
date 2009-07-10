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
#include "userlogc.h"

#include "request.h"

extern int ora_access(test_req_t *req, test_req_t *resp);
extern int bdb_test(const char * backingfile, const char * dbname, char ** rdata, int argc, char * argv[]);

static void bdb_request(test_req_t *req, test_req_t *resp)
{
    char *key, *val;
	char *args[] = {"xatest", "", "", ""};

    key = strtok(req->data, "=");
	val = (key == 0 ? 0 : strtok(NULL, "="));

	args[1] = (val ? "w" : "r");
	args[2] = key;
	args[3] = val;

	if (req->op == 'd' || key == 0)
		args[1] = "d";
#if 0
	(void) bdb_test(req->db, 0, (char **) (&(resp->data)), 4, args);
#endif

	userlogc((char*) "db: %s a1=%s a2=%s a3=%s (rbuf: %s)", req->db, args[1], args[2], args[3], (resp->data != 0 ? resp->data : "null"));
}

static void oracle_test(test_req_t *req, test_req_t *resp)
{
	userlogc((char*) "db=%s data: %s op=%c tx=%d", req->db, req->data, req->op, req->txtype);
	(void) ora_access(req, resp);
}

void DBS(TPSVCINFO * svcinfo) {
	test_req_t *req = (test_req_t *) svcinfo->data;
	test_req_t *resp = (test_req_t *) tpalloc((char*) "X_C_TYPE", (char*) "dc_buf", sizeof (test_req_t));

/*	userlogc((char*) "dbs called  - svc: %s data %s len: %d flags: %d",
 		 svcinfo->name, svcinfo->data, svcinfo->len, svcinfo->flags);*/

/*	buffer = tpalloc("X_OCTET", 0, sendlen);*/
/*	*buffer = 0;*/

	if (req->prod == ORACLE)
		oracle_test(req, resp);
	else if (req->prod == BDB)
		bdb_request(req, resp);
	else {
		snprintf(resp->data, sizeof (resp->data), "Unsupported database: %s", req->db);
		resp->status = 1;
	}

	userlogc((char*) "returning %s", resp->data);
	tpreturn(1, 1, (char *) resp, sizeof (test_req_t), 0);
}
