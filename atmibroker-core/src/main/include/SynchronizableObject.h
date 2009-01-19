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
 */
#ifndef SYNCHRONIZABLEOBJECT_H
#define SYNCHRONIZABLEOBJECT_H

class SynchronizableObject {

public:
	/**
	 * Implementations of this class should implement this method in a platform specific manner.
	 */
	static SynchronizableObject* create(bool recurs);

	/*
	 * This method acquires a lock on the object in order to allow users to perform
	 * execution of code in a thread safe manner.
	 */
	virtual bool lock() = 0;

	/**
	 * This code will wait to be notified or for the specified timeout interval.
	 *
	 * lock MUST be called before executing this method
	 * unlock MUST be called after executing this method
	 */
	virtual bool wait(long timeout) = 0;

	/**
	 * This code will wake up a single thread that is currently in the wait method.
	 *
	 * lock MUST be called before executing this method
	 * unlock MUST be called after executing this method
	 */
	virtual bool notify() = 0;

	/**
	 * This method will release the lock held by the thread on this object
	 */
	virtual bool unlock() = 0;

	/**
	 * This method will check to see if another thread holds the lock on this object.
	 */
	virtual bool tryLock() = 0;
};

#endif
