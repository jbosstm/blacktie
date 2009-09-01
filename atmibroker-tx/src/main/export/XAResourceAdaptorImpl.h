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
#ifndef XARESOURCEADAPTORIMPL_H
#define XARESOURCEADAPTORIMPL_H

#include "CosTransactionsS.h"
#include "atmiBrokerTxMacro.h"
#include "RMException.h"
#include "XAResourceManager.h"

#include <map>

#include "txi.h"
#include "XAStateModel.h"

class XAResourceManager;

class BLACKTIE_TX_DLL XAResourceAdaptorImpl :
	public virtual POA_CosTransactions::Resource, public virtual PortableServer::RefCountServantBase
{
public:
	XAResourceAdaptorImpl(XAResourceManager*, XID&, XID&, CORBA::Long, struct xa_switch_t *) throw (RMException);
	virtual ~XAResourceAdaptorImpl();

    // OTS resource methods
	CosTransactions::Vote prepare() throw (CosTransactions::HeuristicMixed,CosTransactions::HeuristicHazard);
	void rollback() throw(CosTransactions::HeuristicCommit,CosTransactions::HeuristicMixed,CosTransactions::HeuristicHazard);
	void commit() throw(CosTransactions::NotPrepared,CosTransactions::HeuristicRollback,CosTransactions::HeuristicMixed,CosTransactions::HeuristicHazard);
	void commit_one_phase() throw(CosTransactions::HeuristicHazard);
	void forget();

	// has this resource been completed
	bool is_complete();
	void setRecoveryCoordinator(CosTransactions::RecoveryCoordinator_ptr rc) {rc_ = rc;}
	CosTransactions::RecoveryCoordinator_ptr getRecoveryCoordinator() {return rc_;}

	int xa_start (long);
	int xa_end (long);

private:
	XAResourceManager * rm_;
	XID xid_;
	XID bid_;
	bool complete_;
	CORBA::Long rmid_;
	struct xa_switch_t * xa_switch_;
	CosTransactions::RecoveryCoordinator_ptr rc_;
	int flags_;
    int tightly_coupled_;
    atmibroker::xa::XAStateModel sm_;

	void terminate(int) throw(
		CosTransactions::HeuristicRollback,
		CosTransactions::HeuristicMixed,
		CosTransactions::HeuristicHazard);

	int set_flags(int flags);
	void setComplete();
	void notifyError(int, bool);

	// XA methods
	int xa_rollback (long);
	int xa_prepare (long);
	int xa_commit (long);
	int xa_recover (long, long);
	int xa_forget (long);
	int xa_complete (int *, int *, long);
};
#endif // XARESOURCEADAPTORIMPL_H
