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
#ifndef _TX_CONTROL_H
#define _TX_CONTROL_H

#include <vector>
#include "CosTransactionsS.h"
#include "txi.h"

namespace atmibroker {
	namespace tx {

/**
 * An object that gets associated with a thread when there
 * is an active transaction.
 */
class BLACKTIE_TX_DLL TxControl {
public:
	TxControl(CosTransactions::Control_ptr ctrl, int tid);
	virtual ~TxControl();

	int commit(bool report_heuristics);
	int rollback();
	int rollback_only();
	CosTransactions::Status get_ots_status();
	CosTransactions::Control_ptr get_ots_control();	// ref count of ptr is incremented
	int get_status();
	int get_timeout(CORBA::ULong *);

	int thr_id() {return _tid;}
	bool isActive(const char *, bool);
	bool isOriginator();

	// Note this op disassociates the tx and releases _ctrl:
	// perhaps we should make it private and use
	// friend TxManager;
	void suspend();

    // return a list of outstanding xatmi call descriptors associated with this tx
    std::vector<int> &get_cds() {return _cds;}
private:

	int end(bool commit, bool report);

	int _tid;	// ACE thread id
	CosTransactions::Control_ptr _ctrl;
    std::vector<int> _cds;  // xatmi outstanding tpacall descriptors
};
} //	namespace tx
} //namespace atmibroker
#endif	// _TX_CONTROL_H
