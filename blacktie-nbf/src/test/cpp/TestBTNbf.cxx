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
#include "btnbf.h"
#include "xatmi.h"
#include "userlogc.h"

#include "TestAssert.h"
#include "TestBTNbf.h"

void TestBTNbf::test_getattribute() {
	userlogc((char*) "test_getattribute");
	int rc;
	char name[16];
	int len = 16;
	char* buf = (char*)
		"<?xml version='1.0' ?> \
			<employee xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\
				xmlns=\"http://www.jboss.org/blacktie\" \
				xsi:schemaLocation=\"http://www.jboss.org/blacktie buffers/employee.xsd\"> \
				<name>zhfeng</name> \
				<name>test</name> \
			</employee>";

	userlogc((char*) "getattribute of name at index 0");
	rc = btgetattribute(buf, (char*)"name", 0, (char*)&name, &len);
	BT_ASSERT(rc == 0);
	BT_ASSERT(len == 6);
	BT_ASSERT(strcmp(name, "zhfeng") == 0);

	userlogc((char*) "getattribute of name at index 1");
	rc = btgetattribute(buf, (char*)"name", 1, (char*)&name, &len);
	BT_ASSERT(rc == 0);
	BT_ASSERT(len == 4);
	BT_ASSERT(strcmp(name, "test") == 0);
}
