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

log4cxx::LoggerPtr AtmiBrokerSignalHandler::logger_(log4cxx::Logger::getLogger(
		"AtmiBrokerSignalHandler"));
ACE_Sig_Handler AtmiBrokerSignalHandler::handler_;

AtmiBrokerSignalHandler::AtmiBrokerSignalHandler(
	void (*func)(int signum),
	int signum = -1,
	int* signals = NULL, int sigcnt = 0) :
	sigHandler_(func), signum_(signum), guard_(NULL), gCnt_(0), sig_pending_(false) {

	LOG4CXX_DEBUG(logger_, (char*) "AtmiBrokerSignalHandler handler: " << func << " signum: " << signum);
	if (signum_ > 0) {
		ss_.sig_add(signum_);

		handler_.register_handler(signum_, this);
	}

	if (signals != NULL) {
		for (int i = 0; i < sigcnt; i++) {
			LOG4CXX_DEBUG(logger_, (char*) "ignoring signal " << signals[i]);
			handler_.register_handler(signals[i], this);
			ss_.sig_add(signals[i]);
		}
	}
}

AtmiBrokerSignalHandler::~AtmiBrokerSignalHandler() {
	LOG4CXX_DEBUG(logger_, (char*) "~AtmiBrokerSignalHandler");

	if (gCnt_ != 0 || guard_ != NULL) {
		LOG4CXX_WARN(logger_, (char*) "~AtmiBrokerSignalHandler whilst in protected code section");
	}

	if (signum_ > 0)
		handler_.remove_handler(signum_);
}

int AtmiBrokerSignalHandler::handle_signal (int signum, siginfo_t * = 0, ucontext_t * = 0) {

	if (signum == signum_) {
		LOG4CXX_DEBUG(logger_, (char*) "handling signal " << signum);
		if (sigHandler_ != NULL) {
			lock_.lock();
			if (gCnt_ != 0) {
				// received signal whilst another thread is in a protected section
				LOG4CXX_DEBUG(logger_, (char*) "signalled inside protected section: gCnt_=" << gCnt_
					<< " guard_=" << guard_);
				sig_pending_ = true;
			}

			if (!sig_pending_)
				sigHandler_(signum_);
			lock_.unlock();
		}
	} else {
		LOG4CXX_DEBUG(logger_, (char*) "ignoring signal " << signum);
	}

	return 0;	// -1 would unregister the handler
}

void AtmiBrokerSignalHandler::guard() {
	lock_.lock();
	LOG4CXX_DEBUG(logger_, (char*) "Starting protected code section");
	if (guard_ == NULL) {
		LOG4CXX_DEBUG(logger_, (char*) "Block sigset: " << ss_.is_member(signum_));
		guard_ = new ACE_Sig_Guard(&ss_);
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
		if (sig_pending_) {
			LOG4CXX_DEBUG(logger_, (char*) "and running pending sig handler ...");
			sigHandler_(signum_);
			sig_pending_ = false;
		}
	}
	lock_.unlock();
}
