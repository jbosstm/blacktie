/*
 * JBoss, Home of Professional Open Source
 * Copyright 2010, Red Hat, Inc., and others contributors as indicated
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
#include "AtmiBrokerSignalHandler.h"
#include "ThreadLocalStorage.h"
#include "log4cxx/logger.h"
#include "ace/Process.h"

static int default_handlesigs[] = {SIGINT, SIGTERM};
static int default_blocksigs[] = {SIGHUP, SIGALRM, SIGUSR1, SIGUSR2};

log4cxx::LoggerPtr AtmiBrokerSignalHandler::logger_(log4cxx::Logger::getLogger(
		"AtmiBrokerSignalHandler"));
ACE_Sig_Handler AtmiBrokerSignalHandler::handler_;

AtmiBrokerSignalGuard::AtmiBrokerSignalGuard(ACE_Sig_Set& bss) : guard_(NULL), bss_(bss), gCnt_(0), sig_(0) {
}

AtmiBrokerSignalGuard::~AtmiBrokerSignalGuard() {
	if (guard_ != NULL) {
		LOG4CXX_WARN(AtmiBrokerSignalHandler::logger_, (char*) "SIGNAL guard deleted with signals still blocked");

		delete guard_;
	}
}

int AtmiBrokerSignalGuard::blockSignals(bool block) {
	LOG4CXX_DEBUG(AtmiBrokerSignalHandler::logger_, (char*) "SIGNAL: blockSignals: " << gCnt_);

	if (block && guard_ == NULL) {
		LOG4CXX_DEBUG(AtmiBrokerSignalHandler::logger_, (char*) "SIGNAL: blocking sigset");
#ifdef XXX
		guard_ = new ACE_Sig_Guard(&bss_);  // this will disable receipt of selected signals
#else
		guard_ = new ACE_Sig_Guard(NULL);  // this will disable receipt of all signals
#endif
	}

	gCnt_ += 1;

	return gCnt_;
}

int AtmiBrokerSignalGuard::unblockSignals() {
	LOG4CXX_DEBUG(AtmiBrokerSignalHandler::logger_, (char*) "SIGNAL: unblockSignals: " << gCnt_);

	if (gCnt_ == 0) {
		LOG4CXX_WARN(AtmiBrokerSignalHandler::logger_, (char*) "SIGNAL: Unmatched call to unblockSignals.");
	} else if (--gCnt_ == 0) {
		LOG4CXX_DEBUG(AtmiBrokerSignalHandler::logger_, (char*) "SIGNAL: unblocking sigset ");
		if (guard_ != NULL) {
			delete guard_;
			guard_ = NULL;
		}
	}

	return gCnt_;
}

AtmiBrokerSignalHandler::AtmiBrokerSignalHandler(int (*func)(int signum)) {
	init(func,
		default_handlesigs, sizeof (default_handlesigs) / sizeof (int),
		default_handlesigs, sizeof (default_blocksigs) / sizeof (int));
}

AtmiBrokerSignalHandler::AtmiBrokerSignalHandler(
	int (*func)(int signum),
	int* hsignals, int hsigcnt,
	int* bsignals, int bsigcnt) {
	init(func, hsignals, hsigcnt, bsignals,  bsigcnt);
}

void AtmiBrokerSignalHandler::init(int (*func)(int signum), 
	int* hsignals = default_handlesigs, int hsigcnt = sizeof (default_handlesigs) / sizeof (int),
	int* bsignals = default_handlesigs, int bsigcnt = sizeof (default_blocksigs) / sizeof (int)) {

	LOG4CXX_DEBUG(logger_, (char*) "AtmiBrokerSignalHandler handler: " << func << " hsignals: " << hsignals);

	sigHandler_ = func;

	for (int i = 0; i < hsigcnt; i++) {
		LOG4CXX_DEBUG(logger_, (char*) "handling and blocking signal " << hsignals[i]);
		hss_.sig_add(hsignals[i]);
		bss_.sig_add(hsignals[i]);
	}

	for (int i = 0; i < bsigcnt; i++) {
		LOG4CXX_DEBUG(logger_, (char*) "blocking signal " << hsignals[i]);
		bss_.sig_add(bsignals[i]);
	}

	// block all handleable signals when the handler runs
	ACE_Sig_Action sa ((ACE_SignalHandler) 0, bss_, 0);

#ifndef XXX
	for (int i = 1; i < ACE_NSIG; i++)
		if (bss_.is_member(i))
			handler_.register_handler(i, this, &sa);
#else
	for (int i = 1; i < ACE_NSIG; i++)
		if (hss_.is_member(i))
			handler_.register_handler(i, this, &sa);
#endif
}

AtmiBrokerSignalHandler::~AtmiBrokerSignalHandler() {
	LOG4CXX_DEBUG(logger_, (char*) "~AtmiBrokerSignalHandler removing handlers");

	for (int i = 1; i < ACE_NSIG; i++) {
		if (bss_.is_member(i))
			handler_.remove_handler(i);
	}
	LOG4CXX_DEBUG(logger_, (char*) "~AtmiBrokerSignalHandler");
}

/**
 * TODO decide if it would be useful to add an exit handler registration interface:
 *   void addExitHandler(int (*exitHandler)(void))
 * The use case would be to avoid reference counting AtmiBrokerEnv
 *
 * #include "ace/Process_Manager.h"
 * ACE_Process_Manager process_mgr_;
 * process_mgr_.register_handler(this);
 */
int AtmiBrokerSignalHandler::handle_exit(ACE_Process *proc) {
	LOG4CXX_DEBUG(logger_, (char*) "Process " << proc->getpid() <<
		" terminating with status " << proc->return_value());

	return 0;
}

int AtmiBrokerSignalHandler::handle_signal(int signum, siginfo_t * = 0, ucontext_t * = 0) {

	AtmiBrokerSignalGuard* guard = (AtmiBrokerSignalGuard*) getSpecific(TSS_SIG_KEY);
	bool pending = false;
	int rv = 0;

	LOG4CXX_DEBUG(logger_, (char*) "SIGNAL: signum=" << signum << " guard=" << guard);

	// check whether there is a handler for this particular signal
	if (sigHandler_ != NULL && hss_.is_member(signum)) {
		if (guard != NULL && guard->blockCount() > 0) {
			// received signal whilst another thread is in a protected section
			LOG4CXX_DEBUG(logger_, (char*) "SIGNAL: signalled inside protected section");
			pending = true;
		} else {
			LOG4CXX_DEBUG(logger_, (char*) "SIGNAL: handling signal");
			rv = sigHandler_(signum);	// -1 unregisters the handler
		}
	} else {
		LOG4CXX_DEBUG(logger_, (char*) "SIGNAL: ignoring signal");
	}

	if (guard != NULL) {
		LOG4CXX_DEBUG(logger_, (char*) "SIGNAL: updating pending to " << pending);
		guard->lastSignal(signum, pending);
	}

	return rv;
}

void AtmiBrokerSignalHandler::blockSignals(bool sigRestart) {
	AtmiBrokerSignalGuard* guard = (AtmiBrokerSignalGuard*) getSpecific(TSS_SIG_KEY);

	if (guard == NULL) {
		// TODO go through all code and handle the cases where new fails
		if ((guard = new AtmiBrokerSignalGuard(bss_)) == NULL) {
			LOG4CXX_ERROR(logger_, (char*) "Out of memory");
			return;
		}
		setSpecific(TSS_SIG_KEY, guard);
	}

	(void) guard->blockSignals(sigRestart);
}

int AtmiBrokerSignalHandler::unblockSignals() {
	AtmiBrokerSignalGuard* guard = (AtmiBrokerSignalGuard*) getSpecific(TSS_SIG_KEY);
	int sig = 0;

	if (guard == NULL) {
		LOG4CXX_WARN(logger_, (char*) "SIGNAL: unmatched call to unblockSignals");
	} else {
		LOG4CXX_DEBUG(logger_, (char*) "SIGNAL: unblockSignals lastSig=" << guard->lastSignal() <<
			" pending=" << guard->lastPending());

		// has a signal was received since the guarded section of code was entered
		if ((sig = guard->lastSignal()) != 0)
			setSpecific(TPE_KEY, TSS_TPGOTSIG);

		if (guard->unblockSignals() == 0) {
			// see if a handleable signal was received whilst signals were blocked
			if (sigHandler_ != NULL && guard->lastPending() != 0)
				sigHandler_(guard->lastPending());	// run the handler

			destroySpecific(TSS_SIG_KEY);

			delete guard;   // this will re-enable receipt of signals
		}
	}

	return sig;
}
