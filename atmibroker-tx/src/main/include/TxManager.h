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

// CorbaTransaction.h

#ifndef _TXMANAGER_H
#define _TXMANAGER_H

#include "CorbaConnection.h"
#include "TxControl.h"
#include "XAResourceManagerFactory.h"

#define TX_NOT_SUPPORTED   1   /* normal execution */
#define TX_OK              0   /* normal execution */
#define TX_OUTSIDE        -1   /* application is in an RM local
                                  transaction */
#define TX_ROLLBACK       -2   /* transaction was rolled back */
#define TX_MIXED          -3   /* transaction was partially committed
                                  and partially rolled back */
#define TX_HAZARD         -4   /* transaction may have been partially
                                  committed and partially rolled back*/
#define TX_PROTOCOL_ERROR -5   /* routine invoked in an improper
                                  context */
#define TX_ERROR          -6   /* transient error */
#define TX_FAIL           -7   /* fatal error */
#define TX_EINVAL         -8   /* invalid arguments were given */
#define TX_COMMITTED      -9   /* the transaction was heuristically
                                  committed */
#define TX_NO_BEGIN       -100 /* transaction committed plus new
                                  transaction could not be started */
#define TX_ROLLBACK_NO_BEGIN (TX_ROLLBACK+TX_NO_BEGIN)
/* transaction rollback plus new
 transaction could not be started */
#define TX_MIXED_NO_BEGIN (TX_MIXED+TX_NO_BEGIN)
/* mixed plus transaction could not
 be started */
#define TX_HAZARD_NO_BEGIN (TX_HAZARD+TX_NO_BEGIN)
/* hazard plus transaction could not
 be started */
#define TX_COMMITTED_NO_BEGIN (TX_COMMITTED+TX_NO_BEGIN)
/* heuristically committed plus
 transaction could not be started */

/*
 * Definitions for tx_*() routines
 */

/* commit return values */
typedef long COMMIT_RETURN;
#define TX_COMMIT_COMPLETED 0
#define TX_COMMIT_DECISION_LOGGED 1

/* transaction control values */
typedef long TRANSACTION_CONTROL;
#define TX_UNCHAINED 0
#define TX_CHAINED 1

/* type of transaction timeouts */
typedef long TRANSACTION_TIMEOUT;

/* transaction state values */
//typedef long TRANSACTION_STATE;
#define TX_ACTIVE 0
#define TX_TIMEOUT_ROLLBACK_ONLY 1
#define TX_ROLLBACK_ONLY 2

namespace atmibroker {
	namespace tx {

class BLACKTIE_TX_DLL TxManager {
public:
	int open(void);
	int begin(void);
	int commit(void);
	int rollback(void);
	int close(void);
	int rollback_only(void);

	int set_commit_return(COMMIT_RETURN);
	COMMIT_RETURN get_commit_return() {return _whenReturn;}
	int set_transaction_control(TRANSACTION_CONTROL);
	TRANSACTION_CONTROL get_transaction_control() {return _controlMode;}
	int set_transaction_timeout(TRANSACTION_TIMEOUT);
	TRANSACTION_TIMEOUT get_transaction_timeout() {return _timeout;}
	int info(void *); //TXINFO from tx.h
	bool isChained() {return _controlMode == TX_CHAINED;}
	bool reportHeuristics() {return _whenReturn == TX_COMMIT_COMPLETED;}

	TxControl *currentTx(const char *msg);

public:	// public static methods
	static TxManager* get_instance();
	static void discard_instance();
	static CosTransactions::Control_ptr get_ots_control();	// ref count of ptr is incremented

public:	// suspend and resume
	int tx_resume(CosTransactions::Control_ptr control, int flags);
	CosTransactions::Control_ptr tx_suspend(int flags);
	CosTransactions::Control_ptr tx_suspend(TxControl *, int flags);

	int resume(int cd);
	int suspend(int cd);
	bool isCdTransactional(int cd);
	CORBA::ORB_ptr getOrb();

private:
	TxManager();
	virtual ~TxManager();

	int tx_resume(TxControl *, int flags);
	int complete(bool commit);
	int chainTransaction(int);

	int rm_open(void);
	void rm_close(void);
	int rm_start(int flags);
	int rm_end(int flags);

private:
	CORBA_CONNECTION *_connection;
	CosTransactions::TransactionFactory_var _txfac;
	XAResourceManagerFactory _xaRMFac;

	COMMIT_RETURN _whenReturn;
	TRANSACTION_CONTROL _controlMode;
	TRANSACTION_TIMEOUT _timeout;
	bool _isOpen;

private:
	static TxManager *_instance;
};
}	//	namespace tx
}	//namespace atmibroker

#endif // _TXMANAGER_H
