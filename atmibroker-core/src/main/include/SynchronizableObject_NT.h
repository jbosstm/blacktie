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
 * $Id: NT_Mutex.h,v 1.2 2000/03/14 16:40:47 nmcl Exp $
 */

/**
 * This solution uses code made available on:
 * http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
 * In order to provide the pthread_cond_* methods for WIN32
 */
#ifdef WIN32
#ifndef NT_SYNCHRONIZABLEOBJECT_H
#define NT_SYNCHRONIZABLEOBJECT_H

#include "atmiBrokerCoreMacro.h"
#include <windows.h>

#include "SynchronizableObject.h"

typedef HANDLE pthread_mutex_t;

typedef struct {
	int waiters_count_;
	// Number of waiting threads.

	CRITICAL_SECTION waiters_count_lock_;
	// Serialize access to <waiters_count_>.

	HANDLE sema_;
	// Semaphore used to queue up threads waiting for the condition to
	// become signaled.

	HANDLE waiters_done_;
	// An auto-reset event used by the broadcast/signal thread to wait
	// for all the waiting thread(s) to wake up and be released from the
	// semaphore.

	size_t was_broadcast_;
	// Keeps track of whether we were broadcasting or signaling.  This
	// allows us to optimize the code if we're just signaling.
} pthread_cond_t;

class ATMIBROKER_CORE_DLL SynchronizableObject_NT: public SynchronizableObject {
	friend class SynchronizableObject;
public:
	virtual ~SynchronizableObject_NT();

	/*
	 * This method acquires a lock on the object in order to allow users to perform
	 * execution of code in a thread safe manner.
	 */
	virtual bool lock();

	/**
	 * This code will wait to be notified or for the specified timeout interval.
	 *
	 * lock MUST be called before executing this method
	 * unlock MUST be called after executing this method
	 */
	virtual bool wait(long timeout);

	/**
	 * This code will wake up a single thread that is currently in the wait method.
	 *
	 * lock MUST be called before executing this method
	 * unlock MUST be called after executing this method
	 */
	virtual bool notify();

	/**
	 * This method will release the lock held by the thread on this object
	 */
	virtual bool unlock();

	/**
	 * This method will check to see if another thread holds the lock on this object.
	 */
	virtual bool tryLock();
private:
	SynchronizableObject_NT(bool);

	HANDLE mutex; // Windows mutexes are re-entrant.
	bool valid;

	pthread_cond_t cond;
	int pthread_cond_init(pthread_cond_t *cv);
	int pthread_cond_timedwait(pthread_cond_t *cv, pthread_mutex_t *external_mutex, long timeout);
	int pthread_cond_signal(pthread_cond_t *cv);
	int pthread_cond_broadcast(pthread_cond_t *cv);

};

#endif
#endif
