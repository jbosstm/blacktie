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
#include <cppunit/extensions/HelperMacros.h>

#include "TestSynchronizableObject.h"

#include <tao/ORB.h>
#include "userlogc.h"

Waiter::Waiter() {
	object = SynchronizableObject::create(false);
	notified = false;
}

SynchronizableObject* Waiter::getLock() {
	return object;
}

bool Waiter::getNotified() {
	return notified;
}

int Waiter::svc(void){
	object->lock();
	userlogc("waiting");
	object->wait(0);
	userlogc("waited");
	object->unlock();
	notified = true;
	return 0;
}

void TestSynchronizableObject::setUp() {
	int argc = 0;
	CORBA::ORB_ptr orb_ref = CORBA::ORB_init(argc, NULL, "null");
	waiter = new Waiter();
	if (waiter->activate(THR_NEW_LWP| THR_JOINABLE, 1, 0, ACE_DEFAULT_THREAD_PRIORITY, -1, 0, 0, 0, 0, 0, 0) != 0) {
		delete (waiter);
		waiter = NULL;
		CPPUNIT_FAIL("COULD NOT CREATE WAITER");
	}
}
void TestSynchronizableObject::tearDown() {
	if (waiter) {
		waiter->wait();
		delete waiter;
		waiter = NULL;
	}
}

void TestSynchronizableObject::testWaitNotify() {

#ifdef WIN32
	Sleep(1000);
#else
	sleep(1);
#endif
	SynchronizableObject* lock = waiter->getLock();
	lock->lock();
	lock->notify();
	lock->unlock();
	userlogc("done");
	lock->lock();
	bool notified = waiter->getNotified();
	lock->unlock();
	CPPUNIT_ASSERT_MESSAGE("Was not notified", notified == true);
}
