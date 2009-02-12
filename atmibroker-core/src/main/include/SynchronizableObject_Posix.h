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
 * Copyright (C) 1995
 *
 * Arjuna Solutions Limited,
 * Newcastle upon Tyne,
 * Tyne and Wear,
 * UK.
 *
 * $Id: PosixMutex.h,v 1.2 2000/03/14 16:40:34 nmcl Exp $
 */
#ifndef WIN32
#ifndef POSIX_SYNCHRONIZABLEOBJECT_H
#define POSIX_SYNCHRONIZABLEOBJECT_H

#include <pthread.h>

#include "SynchronizableObject.h"

/*
 *
 * POSIX thread system simple mutex.
 *
 */

class SynchronizableObject_Posix: public SynchronizableObject {
	friend class SynchronizableObject;
public:
	virtual ~SynchronizableObject_Posix();

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

private:
	SynchronizableObject_Posix(bool);
	pthread_cond_t cond;
	pthread_mutex_t mutex;
#ifndef HAVE_RECURSIVE_MUTEX
	pthread_t owner;
	int count;
#endif
	bool recursive;
	bool valid;
};

#endif
#endif
