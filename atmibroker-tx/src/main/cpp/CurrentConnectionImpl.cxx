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

#include "CurrentConnectionImpl.h"

#include "log4cxx/logger.h"


log4cxx::LoggerPtr loggerCurrentConnectionImpl(log4cxx::Logger::getLogger("CurrentConnectionImpl"));

CurrentConnectionImpl::CurrentConnectionImpl(LocalResourceManager& localResourceManager) :
	m_localResourceManager(localResourceManager) {
	LOG4CXX_LOGLS(loggerCurrentConnectionImpl, log4cxx::Level::getDebug(), (char*) "constructor");
}

CurrentConnectionImpl::~CurrentConnectionImpl() {
	LOG4CXX_LOGLS(loggerCurrentConnectionImpl, log4cxx::Level::getDebug(), (char*) "destructor");
}

void CurrentConnectionImpl::start(CosTransactions::Coordinator_ptr tx, const CosTransactions::otid_t& aOtid_t) throw(CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentConnectionImpl, log4cxx::Level::getInfo(), (char*) "start");
	XID aXid;
	translateOtidToXid(aOtid_t, aXid);
	//XA::XASwitch aXaSwitch = m_localResourceManager.getResourceManagerDataStruct().xaSwitch;
	XA::XASwitch_ptr aXaSwitch = m_localResourceManager.getResourceManagerDataStruct().xaSwitch;
	//aXaSwitch.xa_start_entry(&aXid, m_localResourceManager.getResourceManagerDataStruct().rmid, TMNOFLAGS);
	aXaSwitch->xa_start(aOtid_t, m_localResourceManager.getResourceManagerDataStruct().rmid, TMNOFLAGS);
	// TODO enlist resource
}

void CurrentConnectionImpl::suspend(CosTransactions::Coordinator_ptr tx, const CosTransactions::otid_t& aOtid_t) throw(CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentConnectionImpl, log4cxx::Level::getInfo(), (char*) "suspend");
	XID aXid;
	translateOtidToXid(aOtid_t, aXid);
	//XA::XASwitch aXaSwitch = m_localResourceManager.getResourceManagerDataStruct().xaSwitch;
	XA::XASwitch_ptr aXaSwitch = m_localResourceManager.getResourceManagerDataStruct().xaSwitch;
	//aXaSwitch.xa_end_entry(&aXid, m_localResourceManager.getResourceManagerDataStruct().rmid, TMSUSPEND);
	aXaSwitch->xa_end(aOtid_t, m_localResourceManager.getResourceManagerDataStruct().rmid, TMSUSPEND);
	// TODO delist resource
}

void CurrentConnectionImpl::resume(CosTransactions::Coordinator_ptr tx, const CosTransactions::otid_t& aOtid_t) throw(CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentConnectionImpl, log4cxx::Level::getInfo(), (char*) "resume");
	XID aXid;
	translateOtidToXid(aOtid_t, aXid);
	//XA::XASwitch aXaSwitch = m_localResourceManager.getResourceManagerDataStruct().xaSwitch;
	XA::XASwitch_ptr aXaSwitch = m_localResourceManager.getResourceManagerDataStruct().xaSwitch;
	//aXaSwitch.xa_start_entry(&aXid, m_localResourceManager.getResourceManagerDataStruct().rmid, TMRESUME);
	aXaSwitch->xa_start(aOtid_t, m_localResourceManager.getResourceManagerDataStruct().rmid, TMRESUME);
	// TODO enlist resource
}

void CurrentConnectionImpl::end(CosTransactions::Coordinator_ptr tx, const CosTransactions::otid_t& aOtid_t, CORBA::Boolean success) throw(CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentConnectionImpl, log4cxx::Level::getInfo(), (char*) "end");
	XID aXid;
	translateOtidToXid(aOtid_t, aXid);
	//XA::XASwitch aXaSwitch = m_localResourceManager.getResourceManagerDataStruct().xaSwitch;
	XA::XASwitch_ptr aXaSwitch = m_localResourceManager.getResourceManagerDataStruct().xaSwitch;
	//aXaSwitch.xa_end_entry(&aXid, m_localResourceManager.getResourceManagerDataStruct().rmid, (success ? TMSUCCESS : TMFAIL));
	aXaSwitch->xa_end(aOtid_t, m_localResourceManager.getResourceManagerDataStruct().rmid, (success ? TMSUCCESS : TMFAIL));
	// TODO delist resource
}

XA::ThreadModel CurrentConnectionImpl::thread_model() throw(CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentConnectionImpl, log4cxx::Level::getInfo(), (char*) "thread_model");
	return m_localResourceManager.getResourceManagerDataStruct().thread_model;
}

CORBA::Long CurrentConnectionImpl::rmid() throw(CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentConnectionImpl, log4cxx::Level::getInfo(), (char*) "rmid");
	return m_localResourceManager.getResourceManagerDataStruct().rmid;
}

void CurrentConnectionImpl::translateOtidToXid(const CosTransactions::otid_t& otid, XID& aXid) {
	LOG4CXX_LOGLS(loggerCurrentConnectionImpl, log4cxx::Level::getInfo(), (char*) "translateOtidToXid");

	aXid.formatID = otid.formatID;
	aXid.bqual_length = otid.bqual_length;
	aXid.gtrid_length = otid.tid.length() - otid.bqual_length;

	int l = otid.tid.length();

	/*
	 if( l > org::xots::context::_XIDDATASIZE ) {
	 ERROR( "unexpected xid data size reported in octet seq. truncate.");
	 l = org::xots::context::_XIDDATASIZE;
	 }
	 */
	for (int idx = 0; idx < l; idx++) {
		aXid.data[idx] = otid.tid[idx];
	}
}

