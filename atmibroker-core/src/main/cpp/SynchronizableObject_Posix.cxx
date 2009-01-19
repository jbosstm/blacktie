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
 * Copyright (C) 1995-1998, 1999, 2000,
 *
 * Arjuna Solutions Limited,
 * Newcastle upon Tyne,
 * Tyne and Wear,
 * UK.
 *
 * $Id: PosixMutex.cc,v 1.3 2000/03/14 16:40:37 nmcl Exp $
 */

/*
 * Implementation of thread ids for Posix threads
 */
#ifndef WIN32

#include <pthread.h>
#include <errno.h>
#include <sys/time.h>

#include "SynchronizableObject_Posix.h"

SynchronizableObject *SynchronizableObject::create(bool recurs) {
	return new SynchronizableObject_Posix(recurs);
}

#ifndef HAVE_RECURSIVE_MUTEX
SynchronizableObject_Posix::SynchronizableObject_Posix(bool recurs) :
	owner(0), count(0), recursive(recurs), valid(false)
#else
SynchronizableObject_Posix::SynchronizableObject_Posix ( bool recurs )
: recursive(recurs),
valid(false)
#endif
{
#if !defined(HAVE_HPUX_PTHREADS) && !defined(HAVE_LYNXOS_PTHREADS) && !defined(HAVE_LINUX_THREADS)
	if (pthread_mutex_init(&mutex, 0) == 0)
		valid = true;
#else
#ifdef HAVE_LINUX_THREADS
	int kind = PTHREAD_MUTEX_RECURSIVE_NP;
#else
	int kind = MUTEX_RECURSIVE_NP;
#endif

	/*
	 * Have recursive mutexes! (Non-portable implementations though!)
	 */

	if (recurs)
	{
		pthread_mutexattr_t attr;

#ifdef HAVE_LINUX_THREADS
		pthread_mutexattr_init(&attr);
#else
		pthread_mutexattr_create(&attr);
#endif
		pthread_mutexattr_setkind_np(&attr, kind);

#ifdef HAVE_LINUX_THREADS
		if (pthread_mutex_init(&mutex, &attr) == 0)
		valid = true;
#else
		if (pthread_mutex_init(&mutex, attr) == 0)
		valid = true;
#endif
	}
	else
	if (pthread_mutex_init(&mutex, pthread_mutexattr_default) == 0)
	valid = true;
#endif
	pthread_cond_init(&cond, NULL);
}

SynchronizableObject_Posix::~SynchronizableObject_Posix() {
	if (valid) {
		pthread_mutex_destroy(&mutex);
	}
}

bool SynchronizableObject_Posix::lock() {
	bool result = false;

	if (valid) {
#ifndef HAVE_RECURSIVE_MUTEX
		pthread_t me = pthread_self();

		if ((recursive) && (pthread_equal(owner, me))) {
			count++;
			result = true;
		} else {
			if (pthread_mutex_lock(&mutex) == 0) {
				count = 1;
				owner = me;
				result = true;
			}
		}
#else
		if (pthread_mutex_lock(&mutex) == 0)
		{
			result = true;
		}
#endif
	}

	return result;
}

bool SynchronizableObject_Posix::wait(long timeout) {
	if (valid) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		struct timespec ts;
		ts.tv_sec = tv.tv_sec;
		ts.tv_nsec = tv.tv_usec * 1000;
		ts.tv_sec += timeout;
		pthread_cond_timedwait(&cond, &mutex, &ts);
		return true;
	} else {
		return false;
	}

}

bool SynchronizableObject_Posix::notify() {
	if (valid) {
		pthread_cond_signal(&cond);
		return true;
	} else {
		return false;
	}

}

bool SynchronizableObject_Posix::unlock() {
	bool result = false;

	if (valid) {
#ifndef HAVE_RECURSIVE_MUTEX
		pthread_t me = pthread_self();

		if (recursive) {
			if (pthread_equal(owner, me)) {
				result = true;

				if (--count == 0) {
					owner = 0;
					count = 0;

					if (pthread_mutex_unlock(&mutex) != 0)
						result = false;
				}
			} else {
				/*
				 * Not the owner, so shouldn't be calling unlock.
				 */
			}
		} else {
			if (pthread_mutex_unlock(&mutex) != 0)
				result = false;
			else
				result = true;
		}
#else
		if (pthread_mutex_unlock(&mutex) != 0)
		result = false;
		else
		result = true;
#endif
	}

	return result;
}

bool SynchronizableObject_Posix::tryLock() {
	bool result = false;

	if (valid) {
#ifndef HAVE_RECURSIVE_MUTEX
		pthread_t me = pthread_self();
		int status = 0;

		if ((recursive) && (pthread_equal(owner, me))) {
			count++;
			result = true;
		} else {
#ifndef HAVE_HPUX_PTHREADS
			if ((status = pthread_mutex_trylock(&mutex)) == 0)
#else
			if ((status = pthread_mutex_trylock(&mutex)) == 1)
#endif
			{
				owner = me;
				count = 1;
				result = true;
			} else if (status != EBUSY)
				valid = false;
		}
#else
#ifndef HAVE_HPUX_PTHREADS
		if (pthread_mutex_trylock(&mutex) == 0)
#else
		if (pthread_mutex_trylock(&mutex) == 1)
#endif
		{
			result = true;
		}
#endif
	}

	return result;
}
#endif
