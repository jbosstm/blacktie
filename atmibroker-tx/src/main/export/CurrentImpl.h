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

#ifndef CurrentImpl_h
#define CurrentImpl_h

#include "atmiBrokerTxMacro.h"

#ifdef TAO_COMP
#include "AtmiBrokerC.h"
#include "CosTransactionsS.h"
#elif ORBIX_COMP
#include "AtmiBroker.hh"
#include <omg/CosTransactions.hh>
#endif
#ifdef VBC_COMP
#include "AtmiBroker_c.hh"
#include "CosTransactions_s.hh"
#endif

#ifdef TAO_COMP
#include "ace/Thread.h"
#endif

#include <deque>

struct ATMIBROKER_TX_DLL _controlThreadStruct {
#ifdef TAO_COMP
	ACE_thread_t thread;
	ACE_hthread_t threadHandle;
#endif
	CosTransactions::Control_var control;
	CORBA::ULong timeout;
};
typedef _controlThreadStruct ControlThreadStruct;

/**
 "local obj" Implementation of CosTransactions::Current interface.
 this class inherits from the stubs, not the skeletons
 because the CosTransactions::Current interface is a "locality contrained" interface
 */
class ATMIBROKER_TX_DLL CurrentImpl: public virtual CosTransactions::Current {
public:
	CurrentImpl(CosTransactions::TransactionFactory* tfac);

	~CurrentImpl(void);

	CurrentImpl(const CurrentImpl &src);
	CurrentImpl& operator=(const CurrentImpl &);
	bool operator==(const CurrentImpl&);

public:
	void begin() throw(CORBA::SystemException, CosTransactions::SubtransactionsUnavailable);

	void commit(CORBA::Boolean report_heuristics) throw (CORBA::SystemException, CosTransactions::NoTransaction, CosTransactions::HeuristicMixed, CosTransactions::HeuristicHazard);

	void rollback() throw (CORBA::SystemException, CosTransactions::NoTransaction);

	void rollback_only() throw (CORBA::SystemException, CosTransactions::NoTransaction);

	CosTransactions::Status get_status() throw (CORBA::SystemException );

	char * get_transaction_name() throw (CORBA::SystemException );

	CORBA::ULong get_timeout() throw(CORBA::SystemException);

	void set_timeout(CORBA::ULong seconds) throw(CORBA::SystemException);

	CosTransactions::Control_ptr get_control() throw(CORBA::SystemException);

	bool remove_control() throw(CORBA::SystemException);

	ControlThreadStruct* get_control_thread_struct() throw(CORBA::SystemException);

	CosTransactions::Control_ptr suspend() throw(CORBA::SystemException);

	void resume(CosTransactions::Control_ptr which) throw(CORBA::SystemException, CosTransactions::InvalidControl);

private:
	CosTransactions::TransactionFactory *m_txfactory;
	CORBA::ULong m_timeout;
	std::deque<ControlThreadStruct*> controlThreadDeque;
};
#endif
