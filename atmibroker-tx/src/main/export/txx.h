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
#ifndef _TXX_H
#define _TXX_H

#include "xa.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * suspend/resume Resource Managers whilst there are outstanding xatmi calls
 */
extern BLACKTIE_TX_DLL int txx_resume(int cd);
extern BLACKTIE_TX_DLL int txx_suspend(int cd);
/**
 * test wether the supplied xatmi call descriptor is transactional
 */
extern BLACKTIE_TX_DLL bool txx_isCdTransactional(int cd);

/**
 * Modify the transaction associated with the target thread such that the only
 * possible outcome of the transaction is to roll back the transaction
 */
extern BLACKTIE_TX_DLL int txx_rollback_only();

/**
 * stop the transaction manager proxy
 */
extern BLACKTIE_TX_DLL void txx_stop(void);

/**
 * Associate a transaction with the current thread:
 * - input parameter 1 is the name of the orb that serialized the IOR passed in the second parameter.
 *   If it is NULL then any orb will be used (and if no orb exists or the orb id is invalid
 *   then a negative return code will be returned)
 * - input parameter 2 is a serialized transaction (ie an IOR)
 *
 * Return a non-negative value on success
 */
extern BLACKTIE_TX_DLL int txx_associate_serialized(char *, char *);

/**
 * Convert the transaction associated with the calling thread into a string.
 * - input parameter 1 is the name of the orb that will perform the serialization.
 *   If it is NULL then any orb will be used (and if no orb exists or the orb id is invalid
 *   then a negative return code will be returned)
 *
 * Return a non-negative value on success
 */
extern BLACKTIE_TX_DLL char* txx_serialize(char *);

/**
 * disassociate a transaction from the current thread
 * (also suspends all Resource Managers linked into the running applications)
 * returns the transaction that was previously associated
 *
 * If the request argument rollback is set to true then the transaction is
 * marked rollback only prior to disassociation from the thread
 *
 * Returns the OTS control associated with the current thread. The caller
 * is responsible for calling release_control on the returned value.
 */
extern BLACKTIE_TX_DLL void * txx_unbind(bool rollback);

/**
 * Return the OTS control associated with the current thread
 * The caller is responsible for calling release_control on the
 * returned control.
 */
extern BLACKTIE_TX_DLL void * txx_get_control();

/**
 * Release an OTS control returned by:
 * get_control
 * txx_unbind
 */
extern BLACKTIE_TX_DLL void txx_release_control(void *);

#ifdef __cplusplus
}
#endif

#endif //_TXX_H
