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
 * Copyright (C) 1994, 1995, 1996, 1997, 1998,
 *
 * Arjuna Solutions Limited,
 * Newcastle upon Tyne,
 * Tyne and Wear,
 * UK.
 *
 * $Id: NT_Mutex.cc,v 1.3 2000/03/14 16:40:49 nmcl Exp $
 */

#ifdef WIN32

#include <iostream>
#include "SynchronizableObject_NT.h"

SynchronizableObject* SynchronizableObject::create(bool recurs) {
	return new SynchronizableObject_NT(recurs);
}

/*
 * Windows mutexes are reentrant. Therefore, we only provide reentrant
 * mutexes currently.
 */

SynchronizableObject_NT::SynchronizableObject_NT(bool reentrant) :
	mutex(0), valid(false) {
	mutex = CreateMutex(NULL, false, NULL);
	pthread_cond_init(&cond);
	valid = true;
}

SynchronizableObject_NT::~SynchronizableObject_NT() {
	CloseHandle(mutex);
}

bool SynchronizableObject_NT::lock() {
	if (valid) {
		if (WaitForSingleObject(mutex, INFINITE) == WAIT_OBJECT_0)
			return true;
	}

	return false;
}

bool SynchronizableObject_NT::wait(long timeout) {
	if (valid) {
		pthread_cond_timedwait(&cond, &mutex, timeout);
		return true;
	} else {
		return false;
	}
}

bool SynchronizableObject_NT::notify() {
	if (valid) {
		pthread_cond_signal(&cond);
		return true;
	} else {
		return false;
	}
}

bool SynchronizableObject_NT::unlock() {
	if (valid) {
		if (ReleaseMutex(mutex))
			return true;
	}

	return false;
}

int SynchronizableObject_NT::pthread_cond_init(pthread_cond_t *cv) {
	cv->waiters_count_ = 0;
	cv->was_broadcast_ = 0;
	cv->sema_ = CreateSemaphore(NULL, // no security
			0, // initially 0
			0x7fffffff, // max count
			NULL); // unnamed
	InitializeCriticalSection(&cv->waiters_count_lock_);
	cv->waiters_done_ = CreateEvent(NULL, // no security
			FALSE, // auto-reset
			FALSE, // non-signaled initially
			NULL); // unnamed
	return 0;
}

int SynchronizableObject_NT::pthread_cond_timedwait(pthread_cond_t *cv,
		pthread_mutex_t *external_mutex, long timeout) {
	// Avoid race conditions.
	EnterCriticalSection(&cv->waiters_count_lock_);
	cv->waiters_count_++;
	LeaveCriticalSection(&cv->waiters_count_lock_);

	// This call atomically releases the mutex and waits on the
	// semaphore until <pthread_cond_signal> or <pthread_cond_broadcast>
	// are called by another thread.
	SignalObjectAndWait(*external_mutex, cv->sema_, timeout, FALSE);

	// Reacquire lock to avoid race conditions.
	EnterCriticalSection(&cv->waiters_count_lock_);

	// We're no longer waiting...
	cv->waiters_count_--;

	// Check to see if we're the last waiter after <pthread_cond_broadcast>.
	int last_waiter = cv->was_broadcast_ && cv->waiters_count_ == 0;

	LeaveCriticalSection(&cv->waiters_count_lock_);

	// If we're the last waiter thread during this particular broadcast
	// then let all the other threads proceed.
	if (last_waiter)
		// This call atomically signals the <waiters_done_> event and waits until
		// it can acquire the <external_mutex>.  This is required to ensure fairness.
		SignalObjectAndWait(cv->waiters_done_, *external_mutex, INFINITE, FALSE);
	else
		// Always regain the external mutex since that's the guarantee we
		// give to our callers.
		WaitForSingleObject(*external_mutex, INFINITE);
	return 0;
}

int SynchronizableObject_NT::pthread_cond_signal(pthread_cond_t *cv) {
	EnterCriticalSection(&cv->waiters_count_lock_);
	int have_waiters = cv->waiters_count_ > 0;
	LeaveCriticalSection(&cv->waiters_count_lock_);

	// If there aren't any waiters, then this is a no-op.
	if (have_waiters)
		ReleaseSemaphore(cv->sema_, 1, 0);

	return 0;
}

int SynchronizableObject_NT::pthread_cond_broadcast(pthread_cond_t *cv) {
	// This is needed to ensure that <waiters_count_> and <was_broadcast_> are
	// consistent relative to each other.
	EnterCriticalSection(&cv->waiters_count_lock_);
	int have_waiters = 0;

	if (cv->waiters_count_ > 0) {
		// We are broadcasting, even if there is just one waiter...
		// Record that we are broadcasting, which helps optimize
		// <pthread_cond_wait> for the non-broadcast case.
		cv->was_broadcast_ = 1;
		have_waiters = 1;
	}

	if (have_waiters) {
		// Wake up all the waiters atomically.
		ReleaseSemaphore(cv->sema_, cv->waiters_count_, 0);

		LeaveCriticalSection(&cv->waiters_count_lock_);

		// Wait for all the awakened threads to acquire the counting
		// semaphore.
		WaitForSingleObject(cv->waiters_done_, INFINITE);
		// This assignment is okay, even without the <waiters_count_lock_> held
		// because no other waiter threads can wake up to access it.
		cv->was_broadcast_ = 0;
	} else
		LeaveCriticalSection(&cv->waiters_count_lock_);
	return 0;
}

#endif
