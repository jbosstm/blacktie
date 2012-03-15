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

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.narayana.blacktie.jatmibroker.jab.JABException;
import org.jboss.narayana.blacktie.jatmibroker.jab.JABMessage;
import org.jboss.narayana.blacktie.jatmibroker.jab.JABServiceInvoker;
import org.jboss.narayana.blacktie.jatmibroker.jab.JABSession;
import org.jboss.narayana.blacktie.jatmibroker.jab.JABTransaction;
import org.jboss.narayana.blacktie.jatmibroker.jab.TransactionException;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.ConnectionException;

/**
 * The JAB connection abstraction allows invocations to services to several services to be multiplexed over the same connection.
 * It also provides a factory method to begin transactions.
 * 
 * @see JABConnectionFactory
 */
public class JABConnection {
    /**
     * The logger to debug using
     */
    private static final Logger log = LogManager.getLogger(JABConnection.class);

    /**
     * The list of open transactions
     */
    private List<Transaction> transactions = new ArrayList<Transaction>();
    private Connection connection;
    private JABSession session;

    private Transaction transaction;

    /**
     * The constructor is hidden from classes as it is intended to be used solely by the JABConnectionFactory class.
     * 
     * @param connection The connection to send requests on
     * @param session The session to use for creating transactions
     * 
     * @throws JABException If the connection cannot be established
     */
    JABConnection(Connection connection, JABSession session) throws JABException {
        this.connection = connection;
        this.session = session;
    }

    /**
     * Begin a new transaction. This transaction reference can then be used in calls to "call" and completed via the options on
     * the JABTransaction class itself.
     * 
     * @param timeout Specify a timeout for the transaction
     * @return The transaction object
     * @throws JABException In case the transaction cannot be created
     */
    public synchronized Transaction beginTransaction(int timeout) throws TransactionException {
        if (transaction != null) {
            return transaction;
        }
        transaction = new Transaction(this, session, timeout);
        return transaction;
    }

    /**
     * Invoke the service specified by the first parameter. The buffer to send is provided as the second parameter. If the
     * service should be invoked within the scope of a transaction, the last parameter should also be provided. This can be
     * obtained via the beginTransaction method.
     * 
     * @param serviceName The name of the service
     * @param toSend The buffer to use
     * @param transaction The transaction to use, may be null
     * @return The buffer returned from the remote service
     * 
     * @throws TransactionException In case the transaction cannot be handled
     * @throws JABException In case the service cannot be contacted
     */
    public synchronized JABResponse call(String serviceName, JABBuffer toSend, Transaction transaction, String bufferType,
            String bufferSubType) throws TransactionException, JABException {
        log.debug("call");

        JABTransaction tx = null;
        if (transaction != null) {
            tx = transaction.getJABTransaction();
        }

        JABServiceInvoker remoteService = new JABServiceInvoker(serviceName, session, bufferType, bufferSubType);
        serialize(toSend, remoteService.getRequest());
        remoteService.call(tx);
        JABResponse responseMessage = new JABResponse(remoteService.getRCode());
        deserialize(responseMessage, remoteService.getResponse());

        return responseMessage;
    }

    /**
     * Shutdown the connection object/
     * 
     * @throws JABException In case associated state cannot be cleaned up
     */
    synchronized void close() throws JABException {
        Iterator<Transaction> iterator = transactions.iterator();
        while (iterator.hasNext()) {
            Transaction next = iterator.next();
            synchronized (next) {
                next.rollback();
            }
            iterator.remove();
        }
        try {
            connection.close();
        } catch (ConnectionException e) {
            throw new JABException("Could not close the connection: " + e.getMessage(), e);
        }
    }

    /**
     * Remove a completed transaction from the list.
     * 
     * @param transaction The completed transaction
     */
    synchronized void removeTransaction(Transaction transaction) {
        transactions.remove(transaction);
    }

    private void serialize(JABBuffer buffer, JABMessage message) {
        Map<String, Class> messageFormat = message.getMessageFormat();
        Iterator<String> iterator = messageFormat.keySet().iterator();
        while (iterator.hasNext()) {
            String key = iterator.next();
            Class type = messageFormat.get(key);
            setMessageValue(buffer, message, key, type);
        }
    }

    private void deserialize(JABBuffer buffer, JABMessage message) {
        Map<String, Class> messageFormat = message.getMessageFormat();
        Iterator<String> iterator = messageFormat.keySet().iterator();
        while (iterator.hasNext()) {
            String key = iterator.next();
            Class type = messageFormat.get(key);
            setBufferValue(buffer, message, key, type);
        }

    }

    private void setBufferValue(JABBuffer buffer, JABMessage message, String key, Class type) {
        try {
            if (type == byte.class) {
                buffer.setValue(key, message.getByte(key));
            } else if (type == short.class) {
                buffer.setValue(key, message.getShort(key));
            } else if (type == int.class) {
                buffer.setValue(key, message.getInt(key));
            } else if (type == long.class) {
                buffer.setValue(key, message.getLong(key));
            } else if (type == double.class) {
                buffer.setValue(key, message.getDouble(key));
            } else if (type == float.class) {
                buffer.setValue(key, message.getFloat(key));
            } else if (type == byte[].class) {
                buffer.setArrayValue(key, message.getByteArray(key));
            } else if (type == short[].class) {
                buffer.setArrayValue(key, message.getShortArray(key));
            } else if (type == int[].class) {
                buffer.setArrayValue(key, message.getIntArray(key));
            } else if (type == long[].class) {
                buffer.setArrayValue(key, message.getLongArray(key));
            } else if (type == double[].class) {
                buffer.setArrayValue(key, message.getDoubleArray(key));
            } else if (type == float[].class) {
                buffer.setArrayValue(key, message.getFloatArray(key));
            }
        } catch (JABException e) {
            log.trace("Could not locate the message property: " + e);
        }
    }

    private void setMessageValue(JABBuffer buffer, JABMessage message, String key, Class type) {
        try {
            if (type == byte.class) {
                message.setByte(key, ((Byte) buffer.getValue(key)));
            } else if (type == short.class) {
                message.setShort(key, ((Short) buffer.getValue(key)));
            } else if (type == int.class) {
                message.setInt(key, ((Integer) buffer.getValue(key)));
            } else if (type == long.class) {
                message.setLong(key, ((Long) buffer.getValue(key)));
            } else if (type == double.class) {
                message.setDouble(key, ((Double) buffer.getValue(key)));
            } else if (type == float.class) {
                message.setFloat(key, ((Float) buffer.getValue(key)));
            } else if (type == byte[].class) {
                message.setByteArray(key, buffer.getByteArray(key));
            } else if (type == short[].class) {
                message.setShortArray(key, buffer.getShortArray(key));
            } else if (type == int[].class) {
                message.setIntArray(key, buffer.getIntArray(key));
            } else if (type == long[].class) {
                message.setLongArray(key, buffer.getLongArray(key));
            } else if (type == double[].class) {
                message.setDoubleArray(key, buffer.getDoubleArray(key));
            } else if (type == float[].class) {
                message.setFloatArray(key, buffer.getFloatArray(key));
            }
        } catch (JABException e) {
            log.trace("Could not locate the message property: " + e);
        }

    }
}
