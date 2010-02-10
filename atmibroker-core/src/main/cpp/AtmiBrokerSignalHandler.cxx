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
#include <stdlib.h>
#include <string.h>

#include "AtmiBrokerSignalHandler.h"
#include "log4cxx/logger.h"

log4cxx::LoggerPtr AtmiBrokerSignalHandler::logger_(log4cxx::Logger::getLogger(
		"AtmiBrokerSignalHandler"));
ACE_Sig_Handler AtmiBrokerSignalHandler::handler_;

AtmiBrokerSignalHandler::AtmiBrokerSignalHandler(
	void (*func)(int signum),
	int* hsignals = NULL, int hsigcnt = 0,
	int* bsignals = NULL, int bsigcnt = 0) :
		sigHandler_(func),
		pending_sig_(0), guard_(NULL), gCnt_(0) {

	LOG4CXX_DEBUG(logger_, (char*) "AtmiBrokerSignalHandler handler: " << func << " hsignals: " << hsignals);

	if (hsigcnt > 0) {
		hsignals_ = (int *) malloc((hsigcnt + 1) * sizeof (int));
		(void) memcpy(hsignals_, hsignals, hsigcnt * sizeof (int));
		hsignals_[hsigcnt] = 0;

		for (int* sigp = hsignals_; *sigp != 0; sigp++) {
			LOG4CXX_DEBUG(logger_, (char*) "handling and blocking signal " << *sigp);

			ss_.sig_add(*sigp);
			handler_.register_handler(*sigp, this);
		}
	} else {
		hsignals_ = NULL;
	}

	if (bsigcnt > 0) {
		bsignals_ = (int *) malloc((bsigcnt + 1) * sizeof (int));
		(void) memcpy(bsignals_, bsignals, bsigcnt * sizeof (int));
		bsignals_[bsigcnt] = 0;

		for (int* sigp = bsignals_; *sigp != 0; sigp++) {
			LOG4CXX_DEBUG(logger_, (char*) "blocking signal " << *sigp);

			ss_.sig_add(*sigp);
		}
	} else {
		bsignals_ = NULL;
	}
}

AtmiBrokerSignalHandler::~AtmiBrokerSignalHandler() {
	LOG4CXX_DEBUG(logger_, (char*) "~AtmiBrokerSignalHandler");

	if (gCnt_ != 0 || guard_ != NULL) {
		LOG4CXX_WARN(logger_, (char*) "~AtmiBrokerSignalHandler whilst in protected code section");
	}

	if (hsignals_ != NULL) {
		for (int* sigp = hsignals_; *sigp != 0; sigp++)
			handler_.remove_handler(*sigp);

		free(hsignals_);
	}

	if (bsignals_ != NULL)
		free(bsignals_);
}

int AtmiBrokerSignalHandler::handle_signal(int signum, siginfo_t * = 0, ucontext_t * = 0) {

	if (hsignals_ != NULL && sigHandler_ != NULL) {
		for (int* sigp = hsignals_; *sigp != 0; sigp++) {
			if (*sigp == signum) {

				LOG4CXX_DEBUG(logger_, (char*) "handling signal " << signum);

				lock_.lock();
				if (gCnt_ != 0) {
					// received signal whilst another thread is in a protected section
					LOG4CXX_DEBUG(logger_, (char*) "signalled inside protected section: gCnt_=" << gCnt_
						<< " guard_=" << guard_);
					pending_sig_ = signum;
				} else {
					sigHandler_(signum);
				}

				lock_.unlock();

				return 0;
			}
		}
	}
			
	LOG4CXX_DEBUG(logger_, (char*) "ignoring signal " << signum);

	return 0;	// -1 would unregister the handler
}

void AtmiBrokerSignalHandler::guard() {
	return; // TODO
	lock_.lock();
	LOG4CXX_DEBUG(logger_, (char*) "Starting protected code section");
	if (guard_ == NULL) {
		LOG4CXX_DEBUG(logger_, (char*) "Block sigset");
		guard_ = new ACE_Sig_Guard(&ss_);
	}

	gCnt_ += 1;
	lock_.unlock();
}

void AtmiBrokerSignalHandler::unguard() {
	return; // TODO
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
			sigHandler_(pending_sig_);
			pending_sig_ = 0;
		}
	}
	lock_.unlock();
}
