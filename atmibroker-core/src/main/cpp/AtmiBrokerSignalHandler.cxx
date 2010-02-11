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
#include "log4cxx/logger.h"
#include "ace/Process.h"

static int default_handlesigs[] = {SIGINT, SIGTERM};
static int default_blocksigs[] = {SIGHUP, SIGALRM, SIGUSR1, SIGUSR2};

log4cxx::LoggerPtr AtmiBrokerSignalHandler::logger_(log4cxx::Logger::getLogger(
		"AtmiBrokerSignalHandler"));
ACE_Sig_Handler AtmiBrokerSignalHandler::handler_;

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
	pending_sig_ = 0;
	guard_ = NULL;
	gCnt_ = 0;

	for (int i = 0; i < hsigcnt; i++) {
		LOG4CXX_DEBUG(logger_, (char*) "handling and blocking signal " << i);
		hss_.sig_add(hsignals[i]);
		bss_.sig_add(hsignals[i]);
		handler_.register_handler(hsignals[i], this);
	}

	for (int i = 0; i < bsigcnt; i++) {
		LOG4CXX_DEBUG(logger_, (char*) "blocking signal " << i);
		bss_.sig_add(bsignals[i]);
		handler_.register_handler(bsignals[i], this);
	}
}

AtmiBrokerSignalHandler::~AtmiBrokerSignalHandler() {
	LOG4CXX_DEBUG(logger_, (char*) "~AtmiBrokerSignalHandler");

	if (gCnt_ != 0 || guard_ != NULL) {
		LOG4CXX_WARN(logger_, (char*) "~AtmiBrokerSignalHandler whilst in protected code section");
	}

	for (int i = 1; i < ACE_NSIG; i++) {
		if (hss_.is_member(i))
			handler_.remove_handler(i);
	}
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

	if (hss_.is_member(signum) && sigHandler_ != NULL) {
		int rv = 0;
		/*
		 * Block further signals whilst the handler runs - BTW this does not mask the signal
		 * for other threads.
		 * TODO decide whether to have the requirement that sigHandler_ must be reentrant.
		 * My vote is YES, handlers should be reentrant.
		 */
		//ACE_Sig_Guard block(&bss_);

		LOG4CXX_DEBUG(logger_, (char*) "handling signal " << signum);

		lock_.lock();
		if (gCnt_ != 0) {
			// received signal whilst another thread is in a protected section
			LOG4CXX_DEBUG(logger_, (char*) "signalled inside protected section: gCnt_=" << gCnt_
				<< " guard_=" << guard_);
			pending_sig_ = signum;
		} else {
			rv = sigHandler_(signum);
		}

		lock_.unlock();

		return rv;	// -1 unregisters the handler
	}
			
	LOG4CXX_DEBUG(logger_, (char*) "ignoring signal " << signum);

	return 0;
}

void AtmiBrokerSignalHandler::guard() {
	lock_.lock();
	LOG4CXX_DEBUG(logger_, (char*) "Starting protected code section");
	if (guard_ == NULL) {
		LOG4CXX_DEBUG(logger_, (char*) "Block sigset");
		guard_ = new ACE_Sig_Guard(&bss_);
	}

	gCnt_ += 1;
	lock_.unlock();
}

void AtmiBrokerSignalHandler::unguard() {
	lock_.lock();
	LOG4CXX_DEBUG(logger_, (char*) "Ending protected code section: gCnt=" << gCnt_);
	if (guard_ == NULL) {
		LOG4CXX_WARN(logger_, (char*) "Trying to exit an unguarded code section.\n");
	} else if (gCnt_ <= 0) {
		LOG4CXX_WARN(logger_, (char*) "Trying to exit an unguarded code section\n");
	} else if (--gCnt_ == 0) {
		LOG4CXX_DEBUG(logger_, (char*) "Unblocking sigset");
		delete guard_;
		guard_ = NULL;
		if (pending_sig_) {
			LOG4CXX_DEBUG(logger_, (char*) "and running pending sig handler ...");
			if (sigHandler_ != NULL)
				sigHandler_(pending_sig_);

			pending_sig_ = 0;
		}
	}
	lock_.unlock();
}
