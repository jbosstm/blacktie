/**
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.codehaus.stomp.jms;

import java.io.ByteArrayOutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import javax.jms.JMSException;
import javax.jms.Destination;
import javax.jms.Message;
import javax.jms.MessageConsumer;
import javax.jms.Session;
import javax.jms.XAConnection;
import javax.jms.XAConnectionFactory;
import javax.jms.XASession;
import javax.naming.InitialContext;
import javax.transaction.Transaction;
import javax.transaction.TransactionManager;
import javax.transaction.xa.XAResource;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.codehaus.stomp.ProtocolException;
import org.codehaus.stomp.Stomp;
import org.codehaus.stomp.StompFrame;
import org.codehaus.stomp.StompFrameError;
import org.codehaus.stomp.StompHandler;
import org.codehaus.stomp.util.IntrospectionSupport;



import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.transaction.InvalidTransactionException;
import javax.transaction.SystemException;
import javax.transaction.Transaction;
import javax.transaction.TransactionManager;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.omg.CosTransactions.Control;
import org.omg.CosTransactions.Unavailable;

import com.arjuna.ats.internal.jta.transaction.jts.AtomicTransaction;
import com.arjuna.ats.internal.jta.transaction.jts.TransactionImple;
import com.arjuna.ats.internal.jts.ControlWrapper;
import com.arjuna.ats.internal.jts.ORBManager;


/**
 * A protocol switch between JMS and Stomp
 *
 * @author <a href="http://people.apache.org/~jstrachan/">James Strachan</a>
 * @author <a href="http://hiramchirino.com">chirino</a>
 */
public class ProtocolConverter implements StompHandler {
    private static final transient Log log = LogFactory.getLog(ProtocolConverter.class);
    private XAConnectionFactory connectionFactory;
    private final StompHandler outputHandler;
    private XAConnection connection;
    private StompSession defaultSession;
    private StompSession clientAckSession;
    private StompSession xaSession;
    private final Map<String,StompSession> transactedSessions = new ConcurrentHashMap<String,StompSession>();
    private final Map subscriptions = new ConcurrentHashMap();
    private final Map<String, MSC> messages = new ConcurrentHashMap<String, MSC>();
	private boolean closing;

	private static TransactionManager tm;

    public ProtocolConverter(XAConnectionFactory connectionFactory, StompHandler outputHandler) throws NamingException{
        this.connectionFactory = connectionFactory;
        this.outputHandler = outputHandler;
		tm = (TransactionManager) new InitialContext().lookup("java:/TransactionManager");
    }

    public XAConnectionFactory getConnectionFactory() {
        return connectionFactory;
    }

    public StompHandler getOutputHandler() {
        return outputHandler;
    }

    private static class JtsTransactionImple extends TransactionImple {

	/**
	 * Construct a transaction based on an OTS control
	 * 
	 * @param wrapper
	 *            the wrapped OTS control
	 */
	public JtsTransactionImple(ControlWrapper wrapper) {
		super(new AtomicTransaction(wrapper));
        putTransaction(this);
	}
    }

	/**
	 * Convert an IOR representing an OTS transaction into a JTA transaction
	 * 
	 * @param orb
	 * 
	 * @param ior
	 *            the CORBA reference for the OTS transaction
	 * @return a JTA transaction that wraps the OTS transaction
	 */
	private static Transaction controlToTx(String ior) {
		log.debug("controlToTx: ior: " + ior);

		ControlWrapper cw = createControlWrapper(ior);
		TransactionImple tx = (TransactionImple) TransactionImple
				.getTransactions().get(cw.get_uid());

		if (tx == null) {
			log.debug("controlToTx: creating a new tx - wrapper: " + cw);
			tx = new JtsTransactionImple(cw);
		}

		return tx;
	}

	private static ControlWrapper createControlWrapper(String ior) {
		org.omg.CORBA.Object obj = ORBManager.getORB().orb()
				.string_to_object(ior);

		Control control = org.omg.CosTransactions.ControlHelper.narrow(obj);
		if (control == null)
			log.warn("createProxy: ior not a control");

		return new ControlWrapper(control);
	}

    public synchronized void close() throws JMSException {
        try {
        	closing = true;
			// PATCHED BY TOM FOR SINGLE MESSAGE DELIVERY
			Iterator<MSC> iterator = messages.values().iterator();
			while (iterator.hasNext()) {
				MSC msc = iterator.next();
				synchronized (msc.consumer) {
					msc.consumer.setMessageListener(null);
					msc.consumer.notify();
				}
			}
            // lets close all the sessions first
            JMSException firstException = null;
            Collection<StompSession> sessions = new ArrayList<StompSession>(transactedSessions.values());
            if (defaultSession != null) {
                sessions.add(defaultSession);
            }
            if (clientAckSession != null) {
                sessions.add(clientAckSession);
            }
            if (xaSession != null) {
            	sessions.add(xaSession);
            }

            for (StompSession session : sessions) {
                try {
                    if (log.isDebugEnabled()) {
                        log.debug("Closing session: " + session + " with ack mode: " + session.getSession().getAcknowledgeMode());
                    }
                    session.close();
                }
                catch (JMSException e) {
                    if (firstException == null) {                                           
                        firstException = e;
                    }
                }
            }

            // now the connetion
            if (connection != null) {
                connection.close();
            }

            if (firstException != null) {
                throw firstException;
            }
        }
        finally {
            connection = null;
            defaultSession = null;
            clientAckSession = null;
            xaSession = null;
            transactedSessions.clear();
            subscriptions.clear();
            messages.clear();
        }
    }

    /**
     * Process a Stomp Frame
     */
    public void onStompFrame(StompFrame command) throws Exception {
        try {
            if (log.isDebugEnabled()) {
                log.debug(">>>> " + command.getAction() + " headers: " + command.getHeaders());
            }
            
            if (command.getClass() == StompFrameError.class) {
                throw ((StompFrameError) command).getException();
            }

            String action = command.getAction();
            if (action.startsWith(Stomp.Commands.SEND)) {
                onStompSend(command);
            }
            else if (action.startsWith(Stomp.Commands.RECEIVE)) {
                onStompReceive(command);
            }
            else if (action.startsWith(Stomp.Commands.ACK)) {
                onStompAck(command);
            }
            else if (action.startsWith(Stomp.Commands.BEGIN)) {
                onStompBegin(command);
            }
            else if (action.startsWith(Stomp.Commands.COMMIT)) {
                onStompCommit(command);
            }
            else if (action.startsWith(Stomp.Commands.ABORT)) {
                onStompAbort(command);
            }
            else if (action.startsWith(Stomp.Commands.SUBSCRIBE)) {
                onStompSubscribe(command);
            }
            else if (action.startsWith(Stomp.Commands.UNSUBSCRIBE)) {
                onStompUnsubscribe(command);
            }
            else if (action.startsWith(Stomp.Commands.CONNECT)) {
                onStompConnect(command);
            }
            else if (action.startsWith(Stomp.Commands.DISCONNECT)) {
                onStompDisconnect(command);
            }
            else {
                throw new ProtocolException("Unknown STOMP action: " + action);
            }
        }
        catch (Exception e) {

            // Let the stomp client know about any protocol errors.
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            PrintWriter stream = new PrintWriter(new OutputStreamWriter(baos, "UTF-8"));
            e.printStackTrace(stream);
            stream.close();

            HashMap headers = new HashMap();
            headers.put(Stomp.Headers.Error.MESSAGE, e.getMessage());

            final String receiptId = (String) command.getHeaders().get(Stomp.Headers.RECEIPT_REQUESTED);
            if (receiptId != null) {
                headers.put(Stomp.Headers.Response.RECEIPT_ID, receiptId);
            }

            StompFrame errorMessage = new StompFrame(Stomp.Responses.ERROR, headers, baos.toByteArray());
            sendToStomp(errorMessage);

            // TODO need to do anything else? Should we close the connection?
        }
    }

    public void onException(Exception e) {
        log.error("Caught: " + e, e);
    }
    
	public boolean addMessageToAck(Message message, MessageConsumer consumer)
			throws JMSException {
		if (!closing) {
			MSC ms = new MSC();
			ms.message = message;
			ms.consumer = consumer;
			messages.put(message.getJMSMessageID(), ms);
		}
		return closing;
	}

    // Implemenation methods
    //-------------------------------------------------------------------------
    protected void onStompConnect(StompFrame command) throws Exception {
        if (connection != null) {
            throw new ProtocolException("Allready connected.");
        }

        final Map headers = command.getHeaders();
        String login = (String) headers.get(Stomp.Headers.Connect.LOGIN);
        String passcode = (String) headers.get(Stomp.Headers.Connect.PASSCODE);
        String clientId = (String) headers.get(Stomp.Headers.Connect.CLIENT_ID);

        XAConnectionFactory factory = getConnectionFactory();
        IntrospectionSupport.setProperties(factory, headers, "factory.");

        if (login != null) {
            connection = factory.createXAConnection(login, passcode);
        }
        else {
            connection = factory.createXAConnection();
        }
        if (clientId != null) {
            connection.setClientID(clientId);
        }
        IntrospectionSupport.setProperties(connection, headers, "connection.");

        connection.start();

        Map responseHeaders = new HashMap();

        responseHeaders.put(Stomp.Headers.Connected.SESSION, connection.getClientID());
        String requestId = (String) headers.get(Stomp.Headers.Connect.REQUEST_ID);
        if (requestId == null) {
            // TODO legacy
            requestId = (String) headers.get(Stomp.Headers.RECEIPT_REQUESTED);
        }
        if (requestId != null) {
            // TODO legacy
            responseHeaders.put(Stomp.Headers.Connected.RESPONSE_ID, requestId);
            responseHeaders.put(Stomp.Headers.Response.RECEIPT_ID, requestId);
        }

        StompFrame sc = new StompFrame();
        sc.setAction(Stomp.Responses.CONNECTED);
        sc.setHeaders(responseHeaders);
        sendToStomp(sc);
    }

    protected void onStompDisconnect(StompFrame command) throws Exception {
        checkConnected();
        close();
    }

	protected void onStompSend(StompFrame command) throws Exception {
		checkConnected();

		Map headers = command.getHeaders();

		String xid = (String) headers.get("messagexid");

		if (xid != null) {
			log.trace("Transaction was propagated: " + xid);
            Transaction tx = controlToTx(xid);
			tm.resume(tx);
			log.trace("Resumed transaction");

			javax.transaction.TransactionManager txMgr = (TransactionManager) new InitialContext()
					.lookup("java:/TransactionManager");
			StompSession session = getXASession();

			XAResource xaRes = ((XASession)session.getSession()).getXAResource();
			Transaction transaction = txMgr.getTransaction();
			log.trace("Got transaction: " + transaction);
			transaction.enlistResource(xaRes);
			log.trace("Enlisted resource");
			
			session.sendToJms(command);
			
			transaction.delistResource(xaRes, XAResource.TMSUCCESS);
			
			log.trace("Delisted resource");
			tm.suspend();
			log.trace("Suspended transaction");
		} else {
			log.trace("WAS NULL XID");

			String stompTx = (String) headers.get(Stomp.Headers.TRANSACTION);

			StompSession session;
			if (stompTx != null) {
				session = getExistingTransactedSession(stompTx);
			} else {
				session = getDefaultSession();
			}

			session.sendToJms(command);
			log.trace("Sent to JMS");
		}
		sendResponse(command);
		log.trace("Sent Response");
	}

	protected void onStompReceive(StompFrame command) throws Exception {
		checkConnected();

		Map headers = command.getHeaders();
		String destinationName = (String) headers.remove(Stomp.Headers.Send.DESTINATION);
		String ior = (String) headers.get("messagexid");
		StompSession session = getSession(headers, ior != null);
		Object o = headers.remove(Stomp.Headers.Send.EXPIRATION_TIME);
		long ttl = (o != null ? Long.parseLong((String) o) : session.getProducer().getTimeToLive());
		Destination destination = session.convertDestination(destinationName, true);
		Message msg;
        MessageConsumer consumer;

		log.trace("Consuming message - ttl=" + ttl + " IOR=" + ior);

		if (ior != null) {
			TransactionManager txMgr = (TransactionManager) new InitialContext()
				.lookup("java:/TransactionManager");

			// resume the transaction
            Transaction tx = controlToTx(ior);
			tm.resume(tx);

			// create an XA consumer
			XASession xaSession = (XASession) session.getSession();
			XAResource xaRes = xaSession.getXAResource();
			consumer = xaSession.createConsumer(destination);

			// make sure the message is transactionally consumed:
			Transaction transaction = txMgr.getTransaction();
			transaction.enlistResource(xaRes);
			msg = (ttl > 0 ? consumer.receive(ttl) : consumer.receive());
			transaction.delistResource(xaRes, XAResource.TMSUSPEND);
			tm.suspend();
		} else {
			javax.jms.Session ss = session.getSession();
			consumer = ss.createConsumer(destination);
			msg = (ttl > 0 ? consumer.receive(ttl) : consumer.receive());
		}

		log.trace("Consumed message: " + msg);
        consumer.close();

		StompFrame sf;

		if (msg == null) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            PrintWriter stream = new PrintWriter(new OutputStreamWriter(baos, "UTF-8"));
			stream.print("No messages available");
            stream.close();

            HashMap eheaders = new HashMap();
            eheaders.put(Stomp.Headers.Error.MESSAGE, "timeout");

            sf = new StompFrame(Stomp.Responses.ERROR, eheaders, baos.toByteArray());
		} else {
			// Don't use sendResponse since it uses Stomp.Responses.RECEIPT as the action
			// which only allows zero length message bodies, Stomp.Responses.MESSAGE is correct:
			sf = session.convertMessage(msg);
		}

		if (headers.containsKey(Stomp.Headers.RECEIPT_REQUESTED))
			sf.getHeaders().put(Stomp.Headers.Response.RECEIPT_ID, headers.get(Stomp.Headers.RECEIPT_REQUESTED));

		sendToStomp(sf);
	}

    protected void onStompBegin(StompFrame command) throws Exception {
        checkConnected();

        Map headers = command.getHeaders();

        String stompTx = (String) headers.get(Stomp.Headers.TRANSACTION);

        if (stompTx == null) {
            throw new ProtocolException("Must specify the transaction you are beginning");
        }

        StompSession session = getTransactedSession(stompTx);
        if (session != null) {
            throw new ProtocolException("The transaction was already started: " + stompTx);
        }
        session = createTransactedSession(stompTx);
        setTransactedSession(stompTx, session);

        sendResponse(command);
    }

    protected void onStompCommit(StompFrame command) throws Exception {
        checkConnected();

        Map headers = command.getHeaders();
        String stompTx = (String) headers.get(Stomp.Headers.TRANSACTION);
        if (stompTx == null) {
            throw new ProtocolException("Must specify the transaction you are committing");
        }

        StompSession session = getExistingTransactedSession(stompTx);
        session.getSession().commit();
        considerClosingTransactedSession(session, stompTx);
        sendResponse(command);
    }

    protected void onStompAbort(StompFrame command) throws Exception {
        checkConnected();
        Map headers = command.getHeaders();

        String stompTx = (String) headers.get(Stomp.Headers.TRANSACTION);
        if (stompTx == null) {
            throw new ProtocolException("Must specify the transaction you are committing");
        }

        StompSession session = getExistingTransactedSession(stompTx);
        session.getSession().rollback();
        considerClosingTransactedSession(session, stompTx);
        sendResponse(command);
    }

    protected void onStompSubscribe(StompFrame command) throws Exception {
        checkConnected();

        Map headers = command.getHeaders();
        String stompTx = (String) headers.get(Stomp.Headers.TRANSACTION);

        StompSession session;
        if (stompTx != null) {
            session = getExistingTransactedSession(stompTx);
        }
        else {
            String ackMode = (String) headers.get(Stomp.Headers.Subscribe.ACK_MODE);

            if (ackMode != null && Stomp.Headers.Subscribe.AckModeValues.CLIENT.equals(ackMode)) {
                session = getClientAckSession();
            }
            else {
                session = getDefaultSession();
            }
        }

        String subscriptionId = (String) headers.get(Stomp.Headers.Subscribe.ID);
        if (subscriptionId == null) {
            subscriptionId = createSubscriptionId(headers);
        }

        StompSubscription subscription = (StompSubscription) subscriptions.get(subscriptionId);
        if (subscription != null) {
            throw new ProtocolException("There already is a subscription for: " + subscriptionId + ". Either use unique subscription IDs or do not create multiple subscriptions for the same destination");
        }
        subscription = new StompSubscription(session, subscriptionId, command);
        subscriptions.put(subscriptionId, subscription);

        sendResponse(command);
    }

    protected void onStompUnsubscribe(StompFrame command) throws Exception {
        checkConnected();
        Map headers = command.getHeaders();

        String destinationName = (String) headers.get(Stomp.Headers.Unsubscribe.DESTINATION);
        String subscriptionId = (String) headers.get(Stomp.Headers.Unsubscribe.ID);

        if (subscriptionId == null) {
            if (destinationName == null) {
                throw new ProtocolException("Must specify the subscriptionId or the destination you are unsubscribing from");
            }
            subscriptionId = createSubscriptionId(headers);
        }

        StompSubscription subscription = (StompSubscription) subscriptions.remove(subscriptionId);
        if (subscription == null) {
            throw new ProtocolException("Cannot unsubscribe as mo subscription exists for id: " + subscriptionId);
        }
        subscription.close();
        sendResponse(command);
    }

    protected void onStompAck(StompFrame command) throws Exception {
        checkConnected();

        // TODO: acking with just a message id is very bogus
        // since the same message id could have been sent to 2 different subscriptions
        // on the same stomp connection. For example, when 2 subs are created on the same topic.

        Map headers = command.getHeaders();
        String messageId = (String) headers.get(Stomp.Headers.Ack.MESSAGE_ID);
        if (messageId == null) {
            throw new ProtocolException("ACK received without a message-id to acknowledge!");
		}

		MSC ms = (MSC) messages.remove(messageId);
		if (ms == null) {
			throw new ProtocolException("No such message for message-id: "
					+ messageId);
		}

		// PATCHED BY TOM FOR SINGLE MESSAGE DELIVERY
		synchronized (ms.consumer) {
			ms.message.acknowledge();
			ms.consumer.notify();
		}
		sendResponse(command);
    }

    protected void checkConnected() throws ProtocolException {
        if (connection == null) {
            throw new ProtocolException("Not connected.");
        }
    }

    /**
     * Auto-create a subscription ID using the destination
     */
    protected String createSubscriptionId(Map headers) {
        return "/subscription-to/" + headers.get(Stomp.Headers.Subscribe.DESTINATION);
    }

    protected StompSession getDefaultSession() throws JMSException {
        if (defaultSession == null) {
            defaultSession = createSession(Session.AUTO_ACKNOWLEDGE);
        }
        return defaultSession;
    }

    protected StompSession getClientAckSession() throws JMSException {
        if (clientAckSession == null) {
            clientAckSession = createSession(Session.CLIENT_ACKNOWLEDGE);
        }
        return clientAckSession;
    }

	protected StompSession getXASession() throws JMSException {
		if (xaSession == null) {
			Session session = connection.createXASession();
			if (log.isDebugEnabled()) {
				log.debug("Created XA session");
			}
			xaSession = new StompSession(this, session);
			log.trace("Created XA Session");
		} else {
			log.trace("Returned existing XA session");
		}
		return xaSession;
	}    

	protected StompSession getSession(Map headers, boolean isXA) throws JMSException, ProtocolException {
		if (isXA) {
			if (xaSession == null) {
				xaSession = new StompSession(this, connection.createXASession());
				log.trace("Created XA Session");
			} else {
				log.trace("Returned existing XA session");
			}

			return xaSession;
		} else {
			String stompTx = (String) headers.get(Stomp.Headers.TRANSACTION);

			return (stompTx == null ? getDefaultSession() : getExistingTransactedSession(stompTx));
		}
	}

    /**
     * Returns the transacted session for the given ID or throws an exception if there is no such session
     */
    protected StompSession getExistingTransactedSession(String stompTx) throws ProtocolException, JMSException {
        StompSession session = getTransactedSession(stompTx);
        if (session == null) {
            throw new ProtocolException("Invalid transaction id: " + stompTx);
        }
        return session;
    }

    protected StompSession getTransactedSession(String stompTx) throws ProtocolException, JMSException {
        return (StompSession) transactedSessions.get(stompTx);
    }

    protected void setTransactedSession(String stompTx, StompSession session) {
        transactedSessions.put(stompTx, session);
    }

    protected StompSession createSession(int ackMode) throws JMSException {
        Session session = connection.createSession(false, ackMode);
        if (log.isDebugEnabled()) {
            log.debug("Created session with ack mode: " + session.getAcknowledgeMode());
        }
        return new StompSession(this, session);
    }

    protected StompSession createTransactedSession(String stompTx) throws JMSException {
        Session session = connection.createSession(true, Session.SESSION_TRANSACTED);
        return new StompSession(this, session);
    }

    protected void sendResponse(StompFrame command) throws Exception {
        final String receiptId = (String) command.getHeaders().get(Stomp.Headers.RECEIPT_REQUESTED);
        // A response may not be needed.
        if (receiptId != null) {
            StompFrame sc = new StompFrame();
            sc.setAction(Stomp.Responses.RECEIPT);
            sc.setHeaders(new HashMap(1));
            sc.getHeaders().put(Stomp.Headers.Response.RECEIPT_ID, receiptId);
            sendToStomp(sc);
        }
    }

    protected void sendToStomp(StompFrame frame) throws Exception {
        if (log.isDebugEnabled()) {
            log.debug("<<<< " + frame.getAction() + " headers: " + frame.getHeaders());
        }
        outputHandler.onStompFrame(frame);
    }

    /**
     * A provider may wish to eagerly close transacted sessions when they are no longer used.
     * Though a better option would be to just time them out after they have no longer been used.
     */
    protected void considerClosingTransactedSession(StompSession session, String stompTx) {
    }
    
	private class MSC {
		public Message message;
		public Session session;
		public MessageConsumer consumer;
	}
}
