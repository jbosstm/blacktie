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
package org.jboss.narayana.blacktie.jatmibroker.jab.factory;

import org.jboss.narayana.blacktie.jatmibroker.jab.JABException;
import org.jboss.narayana.blacktie.jatmibroker.jab.JABSession;
import org.jboss.narayana.blacktie.jatmibroker.jab.JABTransaction;
import org.jboss.narayana.blacktie.jatmibroker.jab.TransactionException;

/**
 * The JABTransaction provides the programmer access to the underlying transaction object it can be committed or rolled back at
 * most once.
 * 
 * @see JABConnection
 */
public class Transaction {

    /**
     * The connection that created the transaction, the transaction must be disassociated from the connection upon completion
     */
    private JABConnection connection;

    /**
     * The real transaction wrapper
     */
    private JABTransaction jabTransaction;

    /**
     * The constructor is hidden from the programmer as it should be created using the factory method beginTransaction of
     * JABConnection
     * 
     * @param connection The connection that initialised this transaction
     * @param timeout The timeout for the transaction
     * @throws JABException In case the transaction cannot be created
     */
    Transaction(JABConnection connection, JABSession session, int timeout) throws TransactionException {
        this.connection = connection;
        try {
            this.jabTransaction = new JABTransaction(session, timeout);
        } catch (Throwable e) {
            throw new TransactionException("Could not create the transaction: " + e.getMessage(), e);
        }
    }

    /**
     * Commit the work performed within the scope of this transaction
     * 
     * @throws JABException In case the transaction cannot be committed
     */
    public synchronized void commit() throws TransactionException {
        jabTransaction.commit();
        connection.removeTransaction(this);
    }

    /**
     * Discard the work associated with this transaction
     * 
     * @throws JABException In case the transaction cannot be rolled back successfully
     */
    public synchronized void rollback() throws TransactionException {
        jabTransaction.rollback();
        connection.removeTransaction(this);
    }

    /**
     * Get the transaction wrapper
     * 
     * @return The transaction wrapper
     */
    JABTransaction getJABTransaction() {
        return jabTransaction;
    }
}
