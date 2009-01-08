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
#include "BaseServerTest.h"
#include "XATMITestSuite.h"
#include "xatmi.h"
#include "tx.h"
#include <string.h>

#include "TestSpecExampleOne.h"

extern void debit_credit_svc(TPSVCINFO *svcinfo);

void TestSpecExampleOne::setUp() {
	// Setup server
	BaseServerTest::setUp();
	BaseServerTest::registerService("TestSpecExampleOne", debit_credit_svc);

	// Do local work
}

void TestSpecExampleOne::tearDown() {
	// Do local work

	// Clean up server
	BaseServerTest::tearDown();
}

/* this test is taken from the XATMI specification */

void TestSpecExampleOne::test_specexampleone() {
	DATA_BUFFER *dptr; /* DATA_BUFFER is a typed buffer of type */
	DATA_BUFFER *cptr; /* X_C_TYPE and subtype dc_buf. The structure */
	long dlen, clen; /* contains a character array named input and an */
	int cd; /* integer named output. */
	/* allocate typed buffers */
	dptr = (DATA_BUFFER *) tpalloc("X_C_TYPE", "dc_buf", 0);
	cptr = (DATA_BUFFER *) tpalloc("X_C_TYPE", "dc_buf", 0);
	/* populate typed buffers with input data */
	strcpy(dptr->input, "debit account 123 by 50");
	strcpy(cptr->input, "credit account 456 by 50");
	tx_begin(); /* start global transaction */
	/* issue asynchronous request to DEBIT, while it is processing... */
	cd = tpacall("DEBIT", (char *) dptr, 0, TPSIGRSTRT);
	/* ...issue synchronous request to CREDIT */
	tpcall("CREDIT", (char *) cptr, 0, (char **) &cptr, &clen, TPSIGRSTRT);
	/* retrieve DEBIT�s reply */
	tpgetrply(&cd, (char **) &dptr, &dlen, TPSIGRSTRT);
	if (dptr->output == OK && cptr->output == OK)
		tx_commit(); /* commit global transaction */
	else
		tx_rollback(); /* rollback global transaction */
}

/* this routine is used for DEBIT and CREDIT */
void debit_credit_svc(TPSVCINFO *svcinfo) {
	DATA_BUFFER *dc_ptr;
	int rval;
	/* extract request typed buffer */
	dc_ptr = (DATA_BUFFER *) svcinfo->data;
	/*
	 * Depending on service name used to invoke this
	 * routine, perform either debit or credit work.
	 */
	if (!strcmp(svcinfo->name, "DEBIT")) {
		/*
		 * Parse input data and perform debit
		 * as part of global transaction.
		 */
	} else {
		/*
		 * Parse input data and perform credit
		 * as part of global transaction.
		 */
	}
	// TODO MAKE TWO TESTS
	if (!dc_ptr->failTest) {
		rval = TPSUCCESS;
		dc_ptr->output = OK;
	} else {
		rval = TPFAIL; /* global transaction will not commit */
		dc_ptr->output = NOT_OK;
	}
	/* send reply and return from service routine */
	tpreturn(rval, 0, (char *) dc_ptr, 0, 0);
}
