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
/* Typed Buffer Functions */
#if 1
#include "TestTPAlloc.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPAlloc);
#include "TestTPTypes.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPTypes);
#include "TestTPFree.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPFree);
#include "TestTPFreeService.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPFreeService);
#include "TestTPRealloc.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPRealloc);
/* Dynamic Service Management */
#include "TestTPUnadvertise.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPUnadvertise);
#include "TestTPAdvertise.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPAdvertise);
/* Request Response */
#include "TestTPCall.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPCall);
#include "TestTPACall.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPACall);
#include "TestTPCancel.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPCancel);
#include "TestTPGetRply.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPGetRply);
/* Service Routing*/
#include "TestTPService.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPService);
#include "TestTPReturn.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPReturn);
/* Conversation */
#include "TestTPConnect.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPConnect);
#include "TestTPDiscon.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPDiscon);
#include "TestTPRecv.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPRecv);
#include "TestTPSend.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPSend);
#include "TestTPConversation.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTPConversation);
/* Examples from the specification */
#include "TestSpecExampleOne.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestSpecExampleOne);
#include "TestSpecExampleTwo.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestSpecExampleTwo);
/* SERVER MANAGEMENT FUNCTIONS */
#include "LoopyServerAndClient.h"
CPPUNIT_TEST_SUITE_REGISTRATION( LoopyServerAndClient);
/* Server Init */
#include "TestServerinit.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestServerinit);
/* Client Init */
#include "TestClientInit.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestClientInit);
#endif

/* Transactional Request Response */
#include "TestTxTPCall.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TestTxTPCall);
#ifndef WIN32
#include "tx/TestTxRMTPCall.h"
//CPPUNIT_TEST_SUITE_REGISTRATION( TestTxRMTPCall);
#else
//TODO
#endif
