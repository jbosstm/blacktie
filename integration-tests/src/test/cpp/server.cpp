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
#include <jni.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include "string.h"

#include "AtmiBrokerServerControl.h"
#include "ace/OS_NS_unistd.h"
#include "xatmi.h"
#include "tx.h"
#include "userlogc.h"
#include "XATMITestSuite.h"

int interationCount = 100;

#include "ace/OS_NS_unistd.h"




#include "xatmi.h"
#include "tx.h"
#include "userlogc.h"


#include "ace/OS_NS_unistd.h"

#include "userlogc.h"

#include "xatmi.h"
#include "tx.h"

extern void test_tpcall_TPETIME_service(TPSVCINFO *svcinfo) {
	int timeout = 21;
	userlogc((char*) "test_tpcall_TPETIME_service, sleeping for %d seconds",
			timeout);
	ACE_OS::sleep(timeout);
	userlogc((char*) "test_tpcall_TPETIME_service, slept for %d seconds",
			timeout);

	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	strcpy(toReturn, "test_tpcall_TPETIME_service");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

extern void test_tpcall_TPEOTYPE_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_TPEOTYPE_service");
	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_C_TYPE", (char*) "test", len);
	strcpy(toReturn, "test_tpcall_TPEOTYPE_service");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

extern void test_tpcall_TPESVCFAIL_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_TPESVCFAIL_service");
	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	strcpy(toReturn, "test_tpcall_TPESVCFAIL_service");
	tpreturn(TPFAIL, 0, toReturn, len, 0);
}

extern void test_tprecv_TPEV_DISCONIMM_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tprecv_TPEV_DISCONIMM_service");
	long rcvlen = 60;
	long revent = 0;
	char* rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen);

	int status = ::tprecv(svcinfo->cd, (char **) &rcvbuf, &rcvlen, (long) 0,
			&revent);
	TXINFO txinfo;
	int inTx = ::tx_info(&txinfo);
	bool rbkOnly = (txinfo.transaction_state == TX_ROLLBACK_ONLY);
	userlogc((char*) "status=%d, inTx=%d, rbkOnly=%d", status, inTx, rbkOnly);
}

extern void test_tprecv_TPEV_SVCFAIL_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tprecv_TPEV_SVCFAIL_service");
	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	strcpy(toReturn, "test_tprecv_TPEV_SVCFAIL_service");
	tpreturn(TPFAIL, 0, toReturn, len, 0);
}

extern void test_no_tpreturn_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_no_tpreturn_service");
}

extern "C"void BAR(TPSVCINFO * svcinfo) {
	int sendlen = 14;
	char* buffer = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strncpy(buffer, "BAR SAYS HELLO", 14);

	tpreturn(TPSUCCESS, 1, buffer, sendlen, 0);
}

extern "C"void tpcall_x_octet(TPSVCINFO * svcinfo) {
	int sendlen = 14;
	char* buffer = tpalloc((char*) "X_OCTET", 0, sendlen);
	strncpy(buffer, "BAR SAYS HELLO", 14);
	tpreturn(TPSUCCESS, 1, buffer, sendlen, 0);
}

void loopy(TPSVCINFO* tpsvcinfo) {
	userlogc((char*) "loopy");
}

/* this routine is used for DEBIT and CREDIT */
void debit_credit_svc(TPSVCINFO *svcinfo) {
	userlogc((char*) "debit_credit_svc: %d", svcinfo->len);
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
	if (dc_ptr->failTest == 0) {
		rval = TPSUCCESS;
		dc_ptr->output = OK;
	} else {
		rval = TPFAIL; /* global transaction will not commit */
		dc_ptr->output = NOT_OK;
	}
	/* send reply and return from service routine */
	tpreturn(rval, 0, (char *) dc_ptr, 0, 0);
	userlogc((char*) "tpreturn 0 hmm: %d", svcinfo->len);
}

/* this routine is used for INQUIRY */
void inquiry_svc(TPSVCINFO *svcinfo) {
	userlogc((char*) "inquiry_svc");
	DATA_BUFFER *ptr;
	long event;
	int rval;
	/* extract initial typed buffer sent as part of tpconnect() */
	ptr = (DATA_BUFFER *) svcinfo->data;
	/*
	 * Parse input string, ptr->input, and retrieve records.
	 * Return 10 records at a time to client. Records are
	 * placed in ptr->output, an array of account records.
	 */
	for (int i = 0; i < 5; i++) {
		/* gather from DBMS next 10 records into ptr->output array */
		tpsend(svcinfo->cd, (char *) ptr, 0, TPSIGRSTRT, &event);
	}
	// TODO DO OK AND FAIL
	if (ptr->failTest == 0) {
		rval = TPSUCCESS;
	} else {
		rval = TPFAIL; /* global transaction will not commit */
	}
	/* terminate service routine, send no data, and */
	/* terminate connection */
	tpreturn(rval, 0, NULL, 0, 0);
}

void testtpacall_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpacall_service");
	int len = 20;
	char *toReturn = (char*) malloc(len);
	strcpy(toReturn, "testtpacall_service");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
	free(toReturn);
}
void test_tpcall_x_octet_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_x_octet_service");
	bool ok = false;
	if (svcinfo->data) {
		if (strncmp(svcinfo->data, "test_tpcall_x_octet", svcinfo->len) == 0) {
			ok = true;
		}
	}

	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	if (ok) {
		strcpy(toReturn, "tpcall_x_octet");
	} else {
		strcpy(toReturn, "fail");
		if (svcinfo->data) {
			strcpy(toReturn, svcinfo->data);
		} else {
			strcpy(toReturn, "dud");
		}
	}
	tpreturn(TPSUCCESS, 20, toReturn, len, 0);
}

void test_tpcall_x_octet_service_zero(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_x_octet_service_zero");
	int len = 0;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	tpreturn(TPSUCCESS, 21, toReturn, len, 0);
}

void test_tpcall_x_common_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_x_common_service");
	bool ok = false;
	DEPOSIT *dptr = (DEPOSIT*) svcinfo->data;
	if (dptr->acct_no == 12345678 && dptr->amount == 50) {
		ok = true;
	} else {
		char* foo = svcinfo->data;
		for (int i = 0; i < svcinfo->len; i++) {
			userlogc((char*) "Position: %d was: %o", i, foo[i]);
		}
		userlogc((char*) "Data was: %d/%d", dptr->acct_no, dptr->amount);
	}

	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	if (ok) {
		strcpy(toReturn, "tpcall_x_common");
	} else {
		strcpy(toReturn, "fail");
	}
	tpreturn(TPSUCCESS, 22, toReturn, len, 0);
}

void test_tpcall_x_c_type_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_x_c_type_service");
	bool ok = false;
	ACCT_INFO *aptr = (ACCT_INFO*) svcinfo->data;
	bool acctEq = aptr->acct_no == 12345678;
	bool nameEq = strcmp(aptr->name, "TOM") == 0;
	bool fooEq = aptr->foo[0] == 1.1F && aptr->foo[1] == 2.2F;
	bool balsEq = aptr->balances[0] == 1.1 && aptr->balances[1] == 2.2;
	if (acctEq && nameEq && fooEq && balsEq) {
		ok = true;
	}
	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	if (ok) {
		strcpy(toReturn, "tpcall_x_c_type");
	} else {
		strcpy(toReturn, "fail");
	}
	tpreturn(TPSUCCESS, 23, toReturn, len, 0);
}
void testtpcancel_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpcancel_service");
	if (!(svcinfo->flags && TPNOREPLY)) {
		int len = 21;
		char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
		strcpy(toReturn, "testtpcancel_service");
		tpreturn(TPSUCCESS, 0, toReturn, len, 0);
	}
}
void testtpconnect_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpconnect_service");
	tpreturn(TPSUCCESS, 0, NULL, 0, 0);
}
void testTPConversation_service(TPSVCINFO *svcinfo) {

	userlogc((char*) "testTPConversation_service ");
	bool fail = false;
	char *sendbuf = ::tpalloc((char*) "X_OCTET", NULL, svcinfo->len);
	char *rcvbuf = ::tpalloc((char*) "X_OCTET", NULL, svcinfo->len);

	char* expectedResult = (char*) malloc(11);
	strcpy(expectedResult, "conversate");
	char* errorMessage = (char*) malloc(10 + 1 + svcinfo->len + 1);
	sprintf(errorMessage, "%s/%s", expectedResult, svcinfo->data);
	if (strncmp(expectedResult, svcinfo->data, 10) != 0) {
		userlogc((char*) "Fail");
		if (svcinfo->data != NULL) {
			userlogc((char*) "Got invalid data");
		} else {
			userlogc((char*) "GOT A NULL");
		}
		fail = true;
	} else {
		long revent = 0;
		userlogc((char*) "Chatting");
		for (int i = 0; i < interationCount; i++) {
			sprintf(sendbuf, "hi%d", i);
			//userlogc((char*) "testTPConversation_service:%s:", sendbuf);
			int result = ::tpsend(svcinfo->cd, sendbuf, svcinfo->len,
					TPRECVONLY, &revent);
			if (result != -1) {
				result = ::tprecv(svcinfo->cd, &rcvbuf, &svcinfo->len, 0,
						&revent);
				if (result == -1 && revent == TPEV_SENDONLY) {
					char* expectedResult = (char*) malloc(svcinfo->len);
					sprintf(expectedResult, "yo%d", i);
					char* errorMessage = (char*) malloc(svcinfo->len * 2 + 1);
					sprintf(errorMessage, "%s/%s", expectedResult, rcvbuf);
					if (strcmp(expectedResult, rcvbuf) != 0) {
						free(expectedResult);
						free(errorMessage);
						fail = true;
						break;
					}
					free(expectedResult);
					free(errorMessage);
				} else {
					fail = true;
					break;
				}
			} else {
				fail = true;
				break;
			}
		}
		userlogc((char*) "Chatted");
	}

	if (fail) {
		tpreturn(TPESVCFAIL, 0, sendbuf, 0, 0);
	} else {
		sprintf(sendbuf, "hi%d", interationCount);
		tpreturn(TPSUCCESS, 0, sendbuf, svcinfo->len, 0);
	}

	::tpfree(rcvbuf);
	free(expectedResult);
	free(errorMessage);
}

void testTPConversation_short_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testTPConversation_short_service");
	long sendlen = 4;
	long revent = 0;
	char *sendbuf = ::tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, "hi0");
	::tpsend(svcinfo->cd, sendbuf, sendlen, 0, &revent);
	strcpy(sendbuf, "hi1");
	tpreturn(TPSUCCESS, 0, sendbuf, sendlen, 0);
}
void testtpdiscon_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpdiscon_service");
}

void testtpfreeservice_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpfreeservice_service");
	// Allocate a buffer to return
	char *toReturn = tpalloc((char*) "X_OCTET", (char*) "acct_info", 1);

	// Free should be idempotent on the inbound buffer
	::tpfree(svcinfo->data);

	// Get the data from tptypes still
	int toTest = ::tptypes(svcinfo->data, NULL, NULL);

	// Check the values of tptypes (should still have been valid
	if (toTest == -1 || tperrno == TPEINVAL) {
		// False
		toReturn[0] = '0';
	} else {
		// True
		toReturn[0] = '1';
	}

	// Return the data
	tpreturn(TPSUCCESS, 0, toReturn, 1, 0);
}
void testtpgetrply_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpgetrply_service");
	char * toReturn = ::tpalloc((char*) "X_OCTET", NULL, 22);
	strcpy(toReturn, "testtpgetrply_service");
	tpreturn(TPSUCCESS, 0, toReturn, 22, 0);
}
void testtprecv_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtprecv_service");
}

void testtpreturn_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpreturn_service");
	char *toReturn = (char*) malloc(21);
	strcpy(toReturn, "testtpreturn_service");
	tpreturn(TPSUCCESS, 0, toReturn, 21, 0);
	free(toReturn);
}

void testtpreturn_service_tpurcode(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpreturn_service_tpurcode");
	int len = 0;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	if (strncmp(svcinfo->data, "24", 2) == 0) {
		tpreturn(TPSUCCESS, 24, toReturn, len, 0);
	} else {
		tpreturn(TPSUCCESS, 77, toReturn, len, 0);
	}
}
void testtpsend_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpsend_service");
}
void testtpservice_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpservice_service");
}
void testtpunadvertise_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpunadvertise_service");
	char * toReturn = new char[26];
	strcpy(toReturn, "testtpunadvertise_service");
	// Changed length from 0L to svcinfo->len
	tpreturn(TPSUCCESS, 0, toReturn, 25, 0);
	delete toReturn;
}
void test_tx_tpcall_x_octet_service_without_tx(TPSVCINFO *svcinfo) {
	userlogc(
			(char*) (char*) "TxLog: service running: test_tx_tpcall_x_octet_service_without_tx");
	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	TXINFO txinfo;
	int inTx = ::tx_info(&txinfo);
	userlogc(
			(char*) (char*) "TxLog: service running: test_tx_tpcall_x_octet_service_without_tx inTx=%d",
			inTx);
	if (inTx == 0) { // or && txinfo.transaction_state != TX_ACTIVE
		strcpy(toReturn, "test_tx_tpcall_x_octet_service_without_tx");
	} else {
		strcpy(toReturn, svcinfo->data);
	}
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void test_tx_tpcall_x_octet_service_with_tx(TPSVCINFO *svcinfo) {
	userlogc(
			(char*) (char*) "TxLog: service running: test_tx_tpcall_x_octet_service_with_tx");
	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	TXINFO txinfo;
	int inTx = ::tx_info(&txinfo);
	userlogc(
			(char*) (char*) "TxLog: service running: test_tx_tpcall_x_octet_service_with_tx inTx=%d",
			inTx);
	if (inTx == 1) { // or && txinfo.transaction_state == TX_ACTIVE
		strcpy(toReturn, "test_tx_tpcall_x_octet_service_with_tx");
	} else {
		strcpy(toReturn, svcinfo->data);
	}
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void test_time_to_live_service(TPSVCINFO *svcinfo) {
	ACE_OS::sleep(45);
	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	strcpy(toReturn, "test_time_to_live_service");

	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_serverinit(JNIEnv *, jobject) {
	int exit_status = -1;
	userlogc((char*) "serverinit called");
#ifdef WIN32
	char* argv[] = {(char*)"server", (char*)"-c", (char*)"win32", (char*)"default", (char*)"-i", (char*)"1"};
#else
	char* argv[] = {(char*)"server", (char*)"-c", (char*)"linux", (char*)"default", (char*)"-i", (char*)"1"};
#endif
	int argc = sizeof(argv)/sizeof(char*);

	exit_status = serverinit(argc, argv);
	userlogc((char*) "serverinit returning");
	return;
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_serverdone(JNIEnv *, jobject) {
	int exit_status = -1;
	userlogc((char*) "serverdone called");
	exit_status = serverdone();
	userlogc((char*) "serverdone returning");
	return;
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseBAR(JNIEnv *, jobject) {
	// Do local work
	tpadvertise((char*) "BAR", BAR);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseLOOPY(JNIEnv *, jobject) {
	tpadvertise((char*) "LOOPY", loopy);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseDEBIT(JNIEnv *, jobject) {
	tpadvertise((char*) "DEBIT", debit_credit_svc);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseCREDIT(JNIEnv *, jobject) {
	tpadvertise((char*) "CREDIT", debit_credit_svc);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseINQUIRY(JNIEnv *, jobject) {
	tpadvertise((char*) "INQUIRY", inquiry_svc);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPACall(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPACall", testtpacall_service);
	// TODO tpadvertise exit_status =
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertisetpcallXOctet(JNIEnv *, jobject) {
	tpadvertise((char*) "tpcall_x_octet", test_tpcall_x_octet_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertisetpcallXOctetZero(JNIEnv *, jobject) {
	tpadvertise((char*) "tpcall_x_octet", test_tpcall_x_octet_service_zero);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertisetpcallXCommon(JNIEnv *, jobject) {
	tpadvertise((char*) "tpcall_x_common", test_tpcall_x_common_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertisetpcallXCType(JNIEnv *, jobject) {
	tpadvertise((char*) "tpcall_x_c_type", test_tpcall_x_c_type_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPCancel(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPCancel", testtpcancel_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPConnect(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPConnect", testtpconnect_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPConversation(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPConversation",
			testTPConversation_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPConversa2(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPConversa2",
			testTPConversation_short_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPDiscon(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPDiscon", testtpdiscon_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPFree(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPFree", testtpfreeservice_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPGetrply(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPGetrply", testtpgetrply_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPRecv(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPRecv", testtprecv_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPReturn(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPReturn", testtpreturn_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPReturn2(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPReturn2",
			testtpreturn_service_tpurcode);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPSend(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPSend", testtpsend_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPService(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPService", testtpservice_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTPUnadvertise(JNIEnv *, jobject) {
	tpadvertise((char*) "TestTPUnadvertise",
			testtpunadvertise_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestTTL(JNIEnv *, jobject) {
	tpadvertise((char*) "TTL",
			test_time_to_live_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTX1(JNIEnv *, jobject) {
	tpadvertise((char*) "tpcall_x_octet", test_tx_tpcall_x_octet_service_without_tx);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTX2(JNIEnv *, jobject) {
	tpadvertise((char*) "tpcall_x_octet", test_tx_tpcall_x_octet_service_with_tx);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestRollbackOnlyTpcallTPETIMEService(JNIEnv *, jobject) {
	tpadvertise((char*) "TestRbkOnly", test_tpcall_TPETIME_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestRollbackOnlyTpcallTPEOTYPEService(JNIEnv *, jobject) {
	tpadvertise((char*) "TestRbkOnly", test_tpcall_TPEOTYPE_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestRollbackOnlyTpcallTPESVCFAILService(JNIEnv *, jobject) {
	tpadvertise((char*) "TestRbkOnly", test_tpcall_TPESVCFAIL_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestRollbackOnlyTprecvTPEVDISCONIMMService(JNIEnv *, jobject) {
	tpadvertise((char*) "TestRbkOnly", test_tprecv_TPEV_DISCONIMM_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestRollbackOnlyTprecvTPEVSVCFAILService(JNIEnv *, jobject) {
	tpadvertise((char*) "TestRbkOnly", test_tprecv_TPEV_SVCFAIL_service);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_tpadvertiseTestRollbackOnlyNoTpreturnService(JNIEnv *, jobject) {
	tpadvertise((char*) "TestRbkOnly", test_no_tpreturn_service);
}

