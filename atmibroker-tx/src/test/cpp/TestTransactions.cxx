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
#include "TestTransactions.h"
#include "tx.h"

void TestTransactions::test_transactions()
{
        CPPUNIT_ASSERT(tx_open() == TX_OK);
        CPPUNIT_ASSERT(tx_begin() == TX_OK);
        CPPUNIT_ASSERT(tx_commit() == TX_OK);
        CPPUNIT_ASSERT(tx_close() == TX_OK);
}
void TestTransactions::test_protocol()
{
	// should not be able to begin, complete or close a transaction before calling tx_open
        CPPUNIT_ASSERT(tx_begin() != TX_OK);
        CPPUNIT_ASSERT(tx_commit() != TX_OK);
        CPPUNIT_ASSERT(tx_rollback() != TX_OK);
        CPPUNIT_ASSERT(tx_close() != TX_OK);

	// open should succeed
        CPPUNIT_ASSERT(tx_open() == TX_OK);
	// should not be able to open a transaction more than once before closing it
        CPPUNIT_ASSERT(tx_open() != TX_OK);
	// should not be able to complete or close a transaction before calling tx_begin
        CPPUNIT_ASSERT(tx_commit() != TX_OK);
        CPPUNIT_ASSERT(tx_rollback() != TX_OK);
        CPPUNIT_ASSERT(tx_close() != TX_OK);

	// begin should succeed
        CPPUNIT_ASSERT(tx_begin() == TX_OK);
	// should not be able to open or close a transaction before calling tx_commit or tx_rollback
        CPPUNIT_ASSERT(tx_close() != TX_OK);
        CPPUNIT_ASSERT(tx_open() != TX_OK);
	// rollback should succeed
        CPPUNIT_ASSERT(tx_rollback() != TX_OK);
	// should not be able to commit or rollback a transaction after it has already completed
        CPPUNIT_ASSERT(tx_commit() != TX_OK);
        CPPUNIT_ASSERT(tx_rollback() != TX_OK);

	// begin should suceed after terminating a transaction
        CPPUNIT_ASSERT(tx_begin() == TX_OK);
        CPPUNIT_ASSERT(tx_commit() == TX_OK);
	// open should fail before calling close
        CPPUNIT_ASSERT(tx_open() != TX_OK);

	// close should succeed
        CPPUNIT_ASSERT(tx_close() == TX_OK);
}
