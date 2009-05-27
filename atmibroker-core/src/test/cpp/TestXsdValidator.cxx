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
#include <cppunit/extensions/HelperMacros.h>

#include "TestXsdValidator.h"
#include "XsdValidator.h"

void TestXsdValidator::test() {
	XsdValidator va;
	bool result;

	result = va.validate(NULL, NULL);
	CPPUNIT_ASSERT(result == false);
	
	result = va.validate("xsd/Server.xsd", "nosuffix");
	CPPUNIT_ASSERT(result == false);

	result = va.validate("nosuffix", "SERVER.xml");
	CPPUNIT_ASSERT(result == false);
	
	result = va.validate("xsd/Server.xsd", "NoSuchFile.xml");
	CPPUNIT_ASSERT(result == false);
	
	result = va.validate("NoSuchSchema.xsd", "SERVER.xml");
	CPPUNIT_ASSERT(result == false);
	
	result = va.validate("xsd/Server.xsd", "SERVER_NONAME.xml");
	CPPUNIT_ASSERT(result == false);

	result = va.validate("xsd/Server.xsd", "SERVER_WRONG.xml");
	CPPUNIT_ASSERT(result == false);
}
