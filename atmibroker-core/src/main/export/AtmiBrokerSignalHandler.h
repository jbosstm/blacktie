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
#ifndef _ATMIBROKERSIGNALHANDLER_H
#define _ATMIBROKERSIGNALHANDLER_H

#include "atmiBrokerCoreMacro.h"
#include "SynchronizableObject.h"

#include "ace/Sig_Handler.h"
#include "ace/Signal.h"

class BLACKTIE_CORE_DLL AtmiBrokerSignalGuard
{
public:
	AtmiBrokerSignalGuard(ACE_Sig_Set& bss);
	~AtmiBrokerSignalGuard();

	int blockCount() { return gCnt_; }
	int lastPending() { return pending_ ? sig_ : 0; }
	void lastSignal(int sig, bool pending) { sig_ = sig; pending_ = pending;}
	int lastSignal() { return sig_; }
	int blockSignals(bool block);
	int unblockSignals();

private:
	ACE_Sig_Guard *guard_;	// the object that implements signal blocking
	ACE_Sig_Set &bss_;	// the set of signals to block
	int gCnt_;  // number of unmatched calls to guard()
	int sig_;	// the last signal received by this thread
	bool pending_;	// and whether the handlers need to be ran
};

class BLACKTIE_CORE_DLL AtmiBrokerSignalHandler : public ACE_Event_Handler
{
public:
	AtmiBrokerSignalHandler(int (*func)(int signum) = NULL);
	AtmiBrokerSignalHandler(int (*func)(int signum), int* hsignals, int hsigcnt, int* bsignals, int bsigcnt);
	virtual ~AtmiBrokerSignalHandler();

	void setSigHandler(int (*sigHandler)(int signum)) { sigHandler_ = sigHandler; }
	virtual int handle_signal (int signum, siginfo_t *, ucontext_t *);
	virtual int handle_exit (ACE_Process *);

	/**
	 * If sigRestart is true start blocking signals (ie system calls will not be interrupted
	 * by the receipt of a signal). Signals will be blocked until the number of
	 * outstanding block calls is cancelled by an equal number of unblock calls.
	 *
	 * If sigRestart is false and a signal is received then any system call is interrupted
	 * and the condition is notified when the matching call to unblockSignals is made.
	 * For XATMI users this translates into tperrno being set to TPEGOTSIG.
	 */
	void blockSignals(bool sigRestart);

	/**
	 * Reverses the effect of @see AtmiBrokerSignalHandler::blockSignals.
	 * @return
	 * 	the signal number of any signal delivered since the protected code block was entered
	 * 	otherwise zero is returned
	 */
	int unblockSignals();

	static log4cxx::LoggerPtr logger_;
private:
	static ACE_Sig_Handler handler_;

	void init(int (*func)(int signum), int* hsignals, int hsigcnt, int* bsignals, int bsigcnt);

private:
//	SynchronizableObject lock_;
	ACE_Sig_Set bss_;	// set of signals to block during guarded sections of code
	ACE_Sig_Set hss_;	// set of signals to handle

	int (*sigHandler_)(int signum);	// the actual signal handler
};

#endif // _ATMIBROKERSIGNALHANDLER_H
