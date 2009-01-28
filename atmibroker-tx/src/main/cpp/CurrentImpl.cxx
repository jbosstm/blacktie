/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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
/*
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT

#include "AtmiBrokerOTS.h"
#include "CurrentImpl.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerCurrentImpl(Logger::getLogger("CurrentImpl"));

CurrentImpl::CurrentImpl(CosTransactions::TransactionFactory* tfac) :
	m_txfactory(tfac), m_timeout(0) {
}

CurrentImpl::~CurrentImpl() {
}

void CurrentImpl::begin() throw(CORBA::SystemException, CosTransactions::SubtransactionsUnavailable ) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "begin ENTERED");

	CosTransactions::Control_var ctrl = get_control();
	if (CORBA::is_nil(ctrl.in())) {
		LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "creating control from Factory");
		ctrl = m_txfactory->create(m_timeout);
		if (CORBA::is_nil(ctrl.in())) {
			CORBA::INTERNAL ex;
			throw ex;
		}
		ControlThreadStruct* aControlThreadStruct = new ControlThreadStruct();
		aControlThreadStruct->control = CosTransactions::Control::_duplicate(ctrl);
		// TODO assuming client
		LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "control " << AtmiBrokerOTS::get_instance()->getOrb()->object_to_string(aControlThreadStruct->control));
		aControlThreadStruct->thread = ACE_Thread::self();
		ACE_Thread::self(aControlThreadStruct->threadHandle);
		LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "thread " << aControlThreadStruct->thread <<
			" threadHandle " << aControlThreadStruct->threadHandle);
		controlThreadDeque.push_back(aControlThreadStruct);
	} else {
		CosTransactions::Coordinator* aCoordinator = ctrl.in()->get_coordinator();
		if (!CORBA::is_nil(aCoordinator)) {
			aCoordinator->create_subtransaction();
		}
	}

	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "begin FINISHED Control " << (void *) ctrl.in());
}

void CurrentImpl::commit(CORBA::Boolean report_heuristics) throw(CORBA::SystemException, CosTransactions::NoTransaction, CosTransactions::HeuristicMixed, CosTransactions::HeuristicHazard ) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "commit ENTERED");

	CosTransactions::Control_var ctrl = get_control();

	if (CORBA::is_nil(ctrl.in())) {
		CosTransactions::NoTransaction ex;
		throw ex;
	} else {
		CosTransactions::Terminator_var term = ctrl->get_terminator();
		LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "got OTS Terminator");
		term->commit(report_heuristics);
		LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "called commit on OTS Terminator");
		remove_control();
	}
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "commit FINISHED");
	return;
}

void CurrentImpl::rollback() throw (CORBA::SystemException, CosTransactions::NoTransaction) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "rollback ENTERED");

	CosTransactions::Control_var cv = get_control();

	if (CORBA::is_nil(cv.in())) {
		CosTransactions::NoTransaction ex;
		throw ex;

	} else {
		CosTransactions::Terminator_var terminator = cv->get_terminator();
		terminator->rollback();
		remove_control();
	}
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "rollback FINISHED");
}

CosTransactions::Status CurrentImpl::get_status() throw (CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "get_status");

	CosTransactions::Control_var cv = get_control();

	if (CORBA::is_nil(cv.in())) {
		return CosTransactions::StatusNoTransaction;
	} else {
		CosTransactions::Coordinator_var coordinator = cv->get_coordinator();
		return (coordinator->get_status());
	}
}

char * CurrentImpl::get_transaction_name() throw (CORBA::SystemException ) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "get_transaction_name ");

	CosTransactions::Control_var cv = get_control();
	if (CORBA::is_nil(cv.in())) {
		return NULL;
	} else {
		CosTransactions::Coordinator_var coordinator = cv->get_coordinator();
		return coordinator->get_transaction_name();
	}
}

CosTransactions::Control_ptr CurrentImpl::get_control() throw (CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "get_control ");

	for (std::deque<ControlThreadStruct*>::iterator itControlThread = controlThreadDeque.begin(); itControlThread != controlThreadDeque.end(); itControlThread++) {
		LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "next itControlThread is: " << (ACE_thread_t)(*itControlThread)->thread);
		if ((*itControlThread)->thread == ACE_Thread::self()) {
			LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "returning control " << (*itControlThread)->control.in());
			return CosTransactions::Control::_duplicate((*itControlThread)->control.in());
		}
	}

	return CosTransactions::Control::_nil();
}

bool CurrentImpl::remove_control() throw (CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "remove_control ");

	bool removed = false;

	for (std::deque<ControlThreadStruct*>::iterator itControlThread = controlThreadDeque.begin(); itControlThread != controlThreadDeque.end(); itControlThread++) {
		LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "next itControlThread is: " << (ACE_thread_t)(*itControlThread)->thread);
		if ((*itControlThread)->thread == ACE_Thread::self())
			controlThreadDeque.erase(itControlThread);
		removed = true;
		return removed;
	}
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getError(), (char*) "no transaction context attached to thread");
	return removed;
}

ControlThreadStruct* CurrentImpl::get_control_thread_struct() throw (CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "get_control_thread_struct ");

	for (std::deque<ControlThreadStruct*>::iterator itControlThread = controlThreadDeque.begin(); itControlThread != controlThreadDeque.end(); itControlThread++) {
		LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "next itControlThread is: " << (ACE_thread_t)(*itControlThread)->thread);
		if ((*itControlThread)->thread == ACE_Thread::self())
			return *itControlThread;
	}
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getError(), (char*) "no transaction context attached to thread");
	return (ControlThreadStruct*) NULL;
}

CosTransactions::Control_ptr CurrentImpl::suspend() throw (CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "suspend STARTED");

	ControlThreadStruct* controlThreadStruct = get_control_thread_struct();

	if (controlThreadStruct == (ControlThreadStruct*) NULL) {
		LOG4CXX_LOGLS(loggerCurrentImpl, Level::getError(), (char*) "no active transaction context for current thread");
		CosTransactions::InvalidControl ex;
		throw ex;

	} else {
		ACE_Thread::suspend(controlThreadStruct->threadHandle);
	}
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "suspend FINISHED");
	return controlThreadStruct->control;
}

void CurrentImpl::resume(CosTransactions::Control_ptr toberesumed) throw (CORBA::SystemException, CosTransactions::InvalidControl) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "resume STARTED");

	ControlThreadStruct* controlThreadStruct = get_control_thread_struct();

	if (controlThreadStruct == (ControlThreadStruct*) NULL) {
		LOG4CXX_LOGLS(loggerCurrentImpl, Level::getError(), (char*) "no active transaction context for current thread");
		CosTransactions::InvalidControl ex;
		throw ex;

	} else {
		ACE_Thread::resume(controlThreadStruct->threadHandle);
	}
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "resume FINISHED");
}

CORBA::ULong CurrentImpl::get_timeout() throw (CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "get_timeout");

	CosTransactions::Control_var cv = get_control();

	if (CORBA::is_nil(cv.in())) {
		LOG4CXX_LOGLS(loggerCurrentImpl, Level::getError(), (char*) "no transaction context attached to thread, returning preset");
		return m_timeout;
	} else {
		CosTransactions::PropagationContext* aPropagationContext = cv.in()->get_coordinator()->get_txcontext();
		return aPropagationContext->timeout;
	}
	return (0);
}

void CurrentImpl::set_timeout(CORBA::ULong seconds) throw (CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "set_timeout " << seconds);

	m_timeout = seconds;

	CosTransactions::Control_var cv = get_control();

	if (CORBA::is_nil(cv.in())) {
		LOG4CXX_LOGLS(loggerCurrentImpl, Level::getError(), (char*) "no transaction context attached to thread, returning preset");
	} else {
		CosTransactions::Coordinator* aCoordinator = cv.in()->get_coordinator();
		if (!CORBA::is_nil(aCoordinator)) {
			CosTransactions::PropagationContext* aPropagationContext = aCoordinator->get_txcontext();
			if (aPropagationContext != (CosTransactions::PropagationContext*) NULL) {
				aPropagationContext->timeout = seconds;
				cv = m_txfactory->recreate(*aPropagationContext);
				LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "propagationContext does exist ...changed time");
			} else
				LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "propagationContext does not exist yet...can't change time to " << seconds);
		} else
			LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "coordinator does not exist yet...can't change time to " << seconds);
	}
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "set_timeout FINISHED");
}

void CurrentImpl::rollback_only() throw (CORBA::SystemException, CosTransactions::NoTransaction) {
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "rollback_only ENTERED");

	CosTransactions::Control_var cv = get_control();
	if (CORBA::is_nil(cv.in())) {
		CosTransactions::NoTransaction ex;
		throw ex;
	}
	if (!CORBA::is_nil(cv.in())) {
		CosTransactions::Coordinator_var coordinator = cv->get_coordinator();
		coordinator->rollback_only();
	}
	LOG4CXX_LOGLS(loggerCurrentImpl, Level::getDebug(), (char*) "rollback_only FINISHED");
}
