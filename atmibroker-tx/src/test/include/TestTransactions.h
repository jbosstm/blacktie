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
#ifndef TestTransactions_H
#define TestTransactions_H

#include <cppunit/extensions/HelperMacros.h>
#include "cppunit/TestFixture.h"
#include <string.h>

class TestTransactions: public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TestTransactions);
//	CPPUNIT_TEST(is_sane);
	CPPUNIT_TEST(test_transactions);
	CPPUNIT_TEST(test_protocol);
	CPPUNIT_TEST(test_info);
	CPPUNIT_TEST(test_timeout);
	CPPUNIT_TEST(test_rollback);
	CPPUNIT_TEST(test_RM);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

//	void is_sane();
	void test_transactions();
	void test_protocol();
	void test_info();
	void test_RM();
	void test_timeout();
	void test_rollback();
	void test_register_resource();
};

#endif
