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
#ifndef _XARESOURCEADAPTORIMPL_H
#define _XARESOURCEADAPTORIMPL_H

#include "XARecoveryLog.h"
#include "XAStateModel.h"
#include "txi.h"

#include <map>
#include <tao/PortableServer/PortableServer.h>
#include "CosTransactionsS.h"
#include "XAResourceManager.h"

using namespace CosTransactions;

class XAResourceManager;

class BLACKTIE_TX_DLL XAResourceAdaptorImpl :
	public virtual POA_CosTransactions::Resource, public virtual PortableServer::RefCountServantBase
{
public:
	XAResourceAdaptorImpl(XAResourceManager*, XID&, XID&, CORBA::Long, struct xa_switch_t *, XARecoveryLog&) throw (RMException);
	virtual ~XAResourceAdaptorImpl();

    // OTS resource methods
	Vote prepare() throw (HeuristicMixed,HeuristicHazard);
	void rollback() throw(HeuristicCommit,HeuristicMixed,HeuristicHazard);
	void commit() throw(NotPrepared,HeuristicRollback,HeuristicMixed,HeuristicHazard);
	void commit_one_phase() throw(HeuristicHazard);
	void forget();

	bool is_complete();	// has this resource finished 2PC - need for testing
	void set_recovery_coordinator(char *rc) {rc_ = rc;}

	int xa_start (long);
	int xa_end (long);

private:
	XAResourceManager * rm_;
	XID xid_;
	XID bid_;
	bool complete_;
	CORBA::Long rmid_;
	struct xa_switch_t * xa_switch_;
	char *rc_;
	int flags_;
    int tightly_coupled_;
    atmibroker::xa::XAStateModel sm_;
	XARecoveryLog& rclog_;
	bool prepared_;

	void terminate(int) throw(
		HeuristicRollback,
		HeuristicMixed,
		HeuristicHazard);

	int set_flags(int flags);
	void set_complete();
	void notify_error(int, bool);

	// XA methods
	int xa_rollback (long);
	int xa_prepare (long);
	int xa_commit (long);
	int xa_recover (long, long);
	int xa_forget (long);
	int xa_complete (int *, int *, long);
};
#endif // _XARESOURCEADAPTORIMPL_H
