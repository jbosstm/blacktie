/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat Middleware LLC, and others contributors as indicated
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

#include "atmiBrokerTxMacro.h"
//#include "TxPolicyS.h"
#include "CosTransactionsS.h"
#include "xa.h"
#include "XAS.h"

class ATMIBROKER_TX_DLL XAResourceAdaptorImpl : public virtual POA_CosTransactions::Resource
{
public:
        XAResourceAdaptorImpl(CORBA::Long, struct xa_switch_t *);
	virtual ~XAResourceAdaptorImpl();

	CosTransactions::Vote prepare();
	void rollback();
	void commit();
	void forget();
	void commit_one_phase();

	// return the resource id
	virtual CORBA::Long rid(void);
	// has this resource been completed
	bool is_complete();

	// XA methods
        char* get_name(); /* name of resource manager */
        long get_flags(); /* resource manager specific options */
        long get_version(); /* must be 0 */
        int xa_open(char *, int, long);
        int xa_close (char *, int, long);
        int xa_start (XID *, int, long);
        int xa_end (XID *, int, long);
        int xa_rollback (XID *, int, long);
        int xa_prepare (XID *, int, long);
        int xa_commit (XID *, int, long);
        int xa_recover (XID *, long, int, long);
        int xa_forget (XID *, int, long);
        int xa_complete (int *, int *, int, long);

private:
	bool complete_;
	const CORBA::Long rid_;
	const struct xa_switch_t * xa_switch_;
	//const XA::XASwitch_ptr xa_switch_;
};
#endif // XARESOURCEADAPTORIMPL_H
