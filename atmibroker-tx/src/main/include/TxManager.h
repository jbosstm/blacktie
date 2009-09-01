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
	int info(TXINFO *);
	bool isChained() {return _controlMode == TX_CHAINED;}
	bool reportHeuristics() {return _whenReturn == TX_COMMIT_COMPLETED;}

	TxControl *currentTx(const char *msg);

public:	// public static methods
	static TxManager* get_instance();
	static void discard_instance();
	static CORBA_CONNECTION* init_orb(char* name);
	static CosTransactions::Control_ptr get_ots_control();	// ref count of ptr is incremented

public:	// suspend and resume
	int tx_resume(CosTransactions::Control_ptr control, int flags);
	CosTransactions::Control_ptr tx_suspend(int flags);
	CosTransactions::Control_ptr tx_suspend(TxControl *, int flags);

    int resume(int cd);
    int suspend(int cd);
    bool isCdTransactional(int cd);

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
