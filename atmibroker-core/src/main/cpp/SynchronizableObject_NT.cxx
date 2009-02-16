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

#include "SynchronizableObject_NT.h"

log4cxx::LoggerPtr SynchronizableObject_NT::logger(log4cxx::Logger::getLogger("SynchronizableObject_NT"));

SynchronizableObject* SynchronizableObject::create(bool recurs) {
	return new SynchronizableObject_NT();
}

SynchronizableObject_NT::SynchronizableObject_NT() :mutex(), cond(mutex) {
}

SynchronizableObject_NT::~SynchronizableObject_NT() {
	LOG4CXX_DEBUG(logger, (char*) "SynchronizableObject_NT released: " << this);
}

bool SynchronizableObject_NT::lock() {
	LOG4CXX_TRACE(logger, (char*) "Acquiring mutex: " << this);
	bool toReturn = mutex.acquire();
	LOG4CXX_TRACE(logger, (char*) "acquired: " << this);
	return toReturn;
}

bool SynchronizableObject_NT::wait(long timeout) {
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

bool SynchronizableObject_NT::notify() {
	LOG4CXX_TRACE(logger, (char*) "Notifying cond: " << this);
	bool toReturn = cond.signal();
	LOG4CXX_TRACE(logger, (char*) "notified: " << this);
	return toReturn;
}

bool SynchronizableObject_NT::unlock() {
	LOG4CXX_TRACE(logger, (char*) "Releasing mutex: " << this);
	bool toReturn = mutex.release();
	LOG4CXX_TRACE(logger, (char*) "Could not release mutex: " << this);
	return toReturn;
}
#endif
