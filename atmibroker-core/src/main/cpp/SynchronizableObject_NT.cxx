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

SynchronizableObject* SynchronizableObject::create(bool recurs) {
	return new SynchronizableObject_NT();
}

/*
 * Windows mutexes are reentrant. Therefore, we only provide reentrant
 * mutexes currently.
 */

SynchronizableObject_NT::SynchronizableObject_NT() :mutex(), cond(mutex) {
}

SynchronizableObject_NT::~SynchronizableObject_NT() {
}

bool SynchronizableObject_NT::lock() {
	return mutex.acquire();
}

bool SynchronizableObject_NT::wait(long timeout) {
	if (timeout > 0) {
		ACE_Time_Value timeoutval(0, (timeout * 1000));
		return cond.wait(&timeoutval);
	} else {
		return cond.wait();
	}
}

bool SynchronizableObject_NT::notify() {
	return cond.signal();
}

bool SynchronizableObject_NT::unlock() {
	bool toReturn = mutex.release();
	Sleep(0);
	return toReturn;
}


#endif
