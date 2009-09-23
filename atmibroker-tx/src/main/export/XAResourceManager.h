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
#ifndef _XARESOURCEMANAGER_H
#define _XARESOURCEMANAGER_H

#include "CorbaConnection.h"
#include "XAResourceAdaptorImpl.h"

class XAResourceAdaptorImpl;

class xid_cmp
{   
public: 
	bool operator()(const XID& xid1, const XID& xid2);
};  

class BLACKTIE_TX_DLL XAResourceManager
{
public:
	XAResourceManager(CORBA_CONNECTION *, const char *, const char *, const char *,
		CORBA::Long, struct xa_switch_t *, XARecoveryLog& log, PortableServer::POA_ptr poa) throw (RMException);
	virtual ~XAResourceManager();

	int xa_start (XID *, long);
	int xa_end (XID *, long);
	int recover(XID& xid, const char* rc);

	// return the resource id
	CORBA::Long rmid(void) {return rmid_;};
	void notify_error(XID *, int, bool);
	void set_complete(XID*);
	const char * name() {return name_;}
	int xa_flags();

	struct xa_switch_t * get_xa_switch() { return xa_switch_;}
	static XID gen_xid(long rmid, XID &gid);

private:
	typedef std::map<XID, XAResourceAdaptorImpl *, xid_cmp> XABranchMap;
	XABranchMap branches_;

	PortableServer::POA_ptr poa_;
	CORBA_CONNECTION* connection_;
	const char *name_;
	const char *openString_;
	const char *closeString_;
	CORBA::Long rmid_;
	struct xa_switch_t * xa_switch_;
	XARecoveryLog& rclog_;

	void createPOA();
	int createServant(XID &);
	XAResourceAdaptorImpl * locateBranch(XID *);

	void show_branches(const char *, XID *);
};
#endif // _XARESOURCEMANAGER_H
