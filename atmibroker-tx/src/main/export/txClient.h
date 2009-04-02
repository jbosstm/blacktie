/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat, Inc., and others contributors as indicated
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
#ifndef _TXCLIENT_H
#define _TXCLIENT_H

#include "tx.h"
#include "CorbaConnection.h"

/**
 * start an orb for making transactional service calls
 * (see orbInit in OrbManagement.h for implementation)
 */
extern ATMIBROKER_TX_DLL CORBA_CONNECTION* startTxOrb(char *);

/**
 * stop the transaction manager proxy
 */
extern ATMIBROKER_TX_DLL void shutdownTxBroker(void);

/**
 * disassociate a transaction from the current thread
 * (also suspends all Resource Managers linked into the running applications)
 * returns the transaction that was previously associated
 */
extern ATMIBROKER_TX_DLL void * disassociateTx(void);
/**
 * associate a transaction with the current thread
 * (also resumes all Resource Managers linked into the running applications)
 */
extern ATMIBROKER_TX_DLL int associateTx(void *);

/**
 * locate an orb by name
 */
extern ATMIBROKER_TX_DLL CORBA::ORB_ptr find_orb(const char *);

/**
 * return the CosTransactions::Control_ptr currently associated wit
 * the calling thread
 */
extern ATMIBROKER_TX_DLL CORBA::Object_ptr current_control();

/**
 * convert a object into an IOR:
 * the first parameter is ptr to a Corba object
 * the second parameter is the name of the orb that owns the object
 */
extern ATMIBROKER_TX_DLL char* txObjectToString(CORBA::Object_ptr, char *);

/**
 * convert an IOR into a Corba object:
 * the input parameter is the name of the orb that owns the IOR
 */
extern ATMIBROKER_TX_DLL CORBA::Object_ptr txStringToObject(char *, char *);

/*
 * using these three methods the caller can convert the current tx control into an IOR:
 * 	txObjectToString(current_control(), "the tx orb")
 *
 * and an ior into a tx control:
 * 	 CORBA::Object_ptr p = txStringToObject("ior", "the tx orb");
 * 	 CosTransactions::Control_ptr cptr = CosTransactions::Control::_narrow(p);
 *
 * (see TxInterceptor for examples)
 */

#endif //_TXCLIENT_H
