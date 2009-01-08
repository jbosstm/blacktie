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
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT

#ifndef TX_H
#define TX_H

#include "atmiBrokerTxMacro.h"

#define TX_NOT_SUPPORTED   1   /* normal execution */
#define TX_OK              0   /* normal execution */
#define TX_OUTSIDE        -1   /* application is in an RM local
                                  transaction */
#define TX_ROLLBACK       -2   /* transaction was rolled back */
#define TX_MIXED          -3   /* transaction was partially committed
                                  and partially rolled back */
#define TX_HAZARD         -4   /* transaction may have been partially
                                  committed and partially rolled back*/
#define TX_PROTOCOL_ERROR -5   /* routine invoked in an improper
                                  context */
#define TX_ERROR          -6   /* transient error */
#define TX_FAIL           -7   /* fatal error */
#define TX_EINVAL         -8   /* invalid arguments were given */
#define TX_COMMITTED      -9   /* the transaction was heuristically
                                  committed */
#define TX_NO_BEGIN       -100 /* transaction committed plus new
                                  transaction could not be started */
#define TX_ROLLBACK_NO_BEGIN (TX_ROLLBACK+TX_NO_BEGIN)
/* transaction rollback plus new
 transaction could not be started */
#define TX_MIXED_NO_BEGIN (TX_MIXED+TX_NO_BEGIN)
/* mixed plus transaction could not
 be started */
#define TX_HAZARD_NO_BEGIN (TX_HAZARD+TX_NO_BEGIN)
/* hazard plus transaction could not
 be started */
#define TX_COMMITTED_NO_BEGIN (TX_COMMITTED+TX_NO_BEGIN)
/* heuristically committed plus
 transaction could not be started */

#ifdef __cplusplus
extern "C" {
#endif
extern ATMIBROKER_TX_DLL int tx_begin(void); // TRANSACTION
extern ATMIBROKER_TX_DLL int tx_close(void); // TRANSACTION
extern ATMIBROKER_TX_DLL int tx_commit(void); // TRANSACTION
extern ATMIBROKER_TX_DLL int tx_open(void); // TRANSACTION
extern ATMIBROKER_TX_DLL int tx_rollback(void); // TRANSACTION
// tx_info
// tx_set_commit_return
// tx_set_transaction_control
// tx_set_transaction_timeout
#ifdef __cplusplus
}
#endif

#endif // END TX_H
