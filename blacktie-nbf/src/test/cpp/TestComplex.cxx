/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
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
#include <stdlib.h>
#include "btnbf.h"
#include "xatmi.h"
#include "userlogc.h"

extern "C" {
#include "AtmiBrokerClientControl.h"
}

#include "TestAssert.h"
#include "TestComplex.h"

void TestComplex::setUp() {
}

void TestComplex::tearDown() {
	::clientdone(0);
}

void TestComplex::test_attribute() {
	userlogc((char*) "test_attribute");
	int rc;
	char name[16];
	//char value[16];
	int len = 0; 

	char* employee = tpalloc((char*)"BT_NBF", (char*)"employee", 0);
	BT_ASSERT(employee != NULL);
	rc = btaddattribute(&employee, (char*)"name", (char*)"zhfeng", 6);	
	BT_ASSERT(rc == 0);
	long id = 1234;
	rc = btaddattribute(&employee, (char*)"id", (char*)&id, sizeof(long));
	BT_ASSERT(rc == 0);

	char* buf = tpalloc((char*)"BT_NBF", (char*)"test", 0);
	BT_ASSERT(buf != NULL);
	rc = btaddattribute(&buf, (char*)"employee", employee, 0);
	printf("%s\n", buf);

	rc = btsetattribute(&employee, (char*)"name", 0, (char*)"tom", 3);
	rc = btaddattribute(&buf, (char*)"employee", employee, 0);
	printf("%s\n", buf);

	rc = btgetattribute(buf, (char*)"employee", 0, employee, &len);
	tpfree(employee);
	tpfree(buf);
}
