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

#include "SynchronizableObject_Posix.h"

log4cxx::LoggerPtr SynchronizableObject_Posix::logger(log4cxx::Logger::getLogger("SynchronizableObject_Posix"));

SynchronizableObject* SynchronizableObject::create(bool recurs) {
	return new SynchronizableObject_Posix();
}

SynchronizableObject_Posix::SynchronizableObject_Posix() :mutex(), cond(mutex) {
}

SynchronizableObject_Posix::~SynchronizableObject_Posix() {
	LOG4CXX_DEBUG(logger, (char*) "SynchronizableObject_Posix released: " << this);
}

bool SynchronizableObject_Posix::lock() {
	LOG4CXX_TRACE(logger, (char*) "Acquiring mutex: " << this);
	bool toReturn = mutex.acquire();
	LOG4CXX_TRACE(logger, (char*) "acquired: " << this);
	return toReturn;
}

bool SynchronizableObject_Posix::wait(long timeout) {
	LOG4CXX_TRACE(logger, (char*) "Waiting for cond: " << this);
	bool toReturn = false;
	if (timeout > 0) {
		ACE_Time_Value timeoutval(0, (timeout * 1000));
		toReturn = cond.wait(&timeoutval);
	} else {
		toReturn = cond.wait();
	}
	LOG4CXX_TRACE(logger, (char*) "waited: " << this);
	return toReturn;
}

bool SynchronizableObject_Posix::notify() {
	LOG4CXX_TRACE(logger, (char*) "Notifying cond: " << this);
	bool toReturn = cond.signal();
	LOG4CXX_TRACE(logger, (char*) "notified: " << this);
	return toReturn;
}

bool SynchronizableObject_Posix::unlock() {
	LOG4CXX_TRACE(logger, (char*) "Releasing mutex: " << this);
	bool toReturn = mutex.release();
	LOG4CXX_TRACE(logger, (char*) "Could not release mutex: " << this);
	return toReturn;
}

#endif
