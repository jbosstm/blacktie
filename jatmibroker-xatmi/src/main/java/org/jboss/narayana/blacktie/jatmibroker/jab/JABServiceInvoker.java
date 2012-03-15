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
package org.jboss.narayana.blacktie.jatmibroker.jab;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.Response;

/**
 * Create an invoker for a remote service. It must be constructed using a JABSession and the name of the service to invoke.
 * 
 * @see JABSession
 */
public class JABServiceInvoker {
    /**
     * The logger to debug using
     */
    private static final Logger log = LogManager.getLogger(JABServiceInvoker.class);

    /**
     * The real connection to the service
     */
    private Connection connection;

    /**
     * The name of the service to invoke
     */
    private String serviceName;

    /**
     * The buffer to send.
     */
    private JABMessage requestMessage;

    /**
     * The buffer to send.
     */
    private JABMessage responseMessage;

    /**
     * Should the service wait forever.
     */
    boolean noTimeout;

    /**
     * The remote service constructor allows the programmer to access a remote XATMI service.
     * 
     * @param aServiceName The name of the service to invoke
     * @param aJABSession The session to use
     * @throws JABException In case the remote service cannot be accessed.
     */
    public JABServiceInvoker(String aServiceName, JABSession aJABSession, String bufferType, String bufferSubType)
            throws JABException {
        log.debug("JABService constructor");

        connection = aJABSession.getConnection();
        serviceName = aServiceName;
        requestMessage = new JABMessage(connection, bufferType, bufferSubType);
    }

    /**
     * Call the remote service within the scope of the transaction specified in the signature of the invocation.
     * 
     * @param tx The transactional scoping. The current transaction is suspended if tx is null or not equal to the current
     *        transaction. If tx is not null and not equal to the current transaction then it is resumed. The transaction to
     *        thread association is restored after the method returns.
     * 
     * @throws TransactionException If the transaction cannot be handled
     * @throws JABException If the call cannot be issued.
     */
    public void call(JABTransaction tx) throws TransactionException, JABException {
        log.debug("JABService call");
        JABTransaction prev = null;

        try {
            if (tx == null) {
                log.debug("service_request tx is null");
                prev = JABTransaction.suspend();
            } else if (!tx.equals(JABTransaction.current())) {
                log.debug("service_request suspend " + prev + " resume " + tx);
                prev = JABTransaction.suspend();
                JABTransaction.resume(tx);
            } else {
                log.debug("service_request tx same as current");
            }

            Buffer request = requestMessage.getBuffer();
            log.debug("service_request tpcall");
            Response response = connection.tpcall(serviceName, request, noTimeout ? Connection.TPNOTIME : 0);
            responseMessage = new JABMessage(response);
            log.debug("service_request responsed");
        } catch (Exception e) {
            log.warn("service_request exception: " + e.getMessage(), e);
            throw new JABException("Could not send tpcall", e);
        } finally {
            if (prev != null) {
                if (tx != null) {
                    log.debug("service_request resp: suspending current: " + JABTransaction.current());
                    JABTransaction.suspend();
                }

                log.debug("service_request resuming prev: " + prev);
                JABTransaction.resume(prev);
            }
        }
    }

    /**
     * Clear the request and response buffers prior to a re-invocation.
     */
    public void clear() {
        log.debug("JABService clear");
        requestMessage.clear();
        responseMessage = null;
    }

    /**
     * Do not timeout the request.
     * 
     * @param noTimeout Do not stop waiting for responses.
     */
    public void setNoTimeout(boolean noTimeout) {
        this.noTimeout = noTimeout;
    }

    /**
     * Get the content of the outbound buffer.
     * 
     * @return The requests input
     */
    public JABMessage getRequest() {
        return requestMessage;
    }

    /**
     * Get the content of the response message
     * 
     * @return The requests output
     */
    public JABMessage getResponse() {
        return responseMessage;
    }

    /**
     * Get the rcode that tpreturn returned with.
     * 
     * @return The application return code.
     */
    public int getRCode() {
        if (responseMessage != null) {
            return responseMessage.getRCode();
        } else {
            return -1;
        }
    }
}
