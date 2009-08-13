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
package org.jboss.blacktie.jatmibroker.xatmi;

import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.AtmiBrokerClientXML;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.JtsTransactionImple;
import org.jboss.blacktie.jatmibroker.core.transport.Message;
import org.jboss.blacktie.jatmibroker.core.transport.Sender;
import org.jboss.blacktie.jatmibroker.core.transport.Transport;
import org.jboss.blacktie.jatmibroker.core.transport.TransportFactory;
import org.jboss.blacktie.jatmibroker.jab.JABException;
import org.jboss.blacktie.jatmibroker.jab.JABTransaction;

/**
 * All services should extend this class as it provides the core service
 * template method.
 */
public abstract class Service implements BlacktieService {
	/**
	 * The logger to use.
	 */
	private static final Logger log = LogManager.getLogger(Service.class);

	/**
	 * The transport to use.
	 */
	private Transport transport;

	/**
	 * The service needs the name of the service so that it can be resolved in
	 * the Environment.xml file
	 * 
	 * @param name
	 *            The name of the service
	 * @throws ConfigurationException
	 * @throws ConnectionException
	 */
	public Service(String name) throws ConfigurationException,
			ConnectionException {
		Properties properties;
		AtmiBrokerClientXML xml = new AtmiBrokerClientXML();
		properties = xml.getProperties();
		transport = TransportFactory.loadTransportFactory(name, properties)
				.createTransport();
		log.debug("Service created: " + name);
	}

	/**
	 * Clean up resources for this service
	 * 
	 * @throws ConnectionException
	 */
	public void close() throws ConnectionException {
		transport.close();
	}

	/**
	 * Entry points should pass control to this method as soon as reasonably
	 * possible.
	 * 
	 * @param message
	 *            The message to process
	 * @throws ConnectionException
	 *             In case communication fails
	 */
	protected void processMessage(Message message) throws ConnectionException {
		if (message.control != null) {
			try {
				JABTransaction.associateTx(message.control); // associate tx
				// with current
				// thread
			} catch (JABException e) {
				log.warn("Got an invalid tx from queue: " + e);
			}
		}
		if (JtsTransactionImple.hasTransaction()) {
			log
					.error("Blacktie MDBs must not be called with a transactional context");
		} else {
			log.trace("Service invoked");
		}
		log.trace("obtained transport");
		Sender sender = null;
		boolean hasTPNOREPLY = (message.flags & Connection.TPNOREPLY) == Connection.TPNOREPLY;
		if (!hasTPNOREPLY && message.replyTo != null
				&& !message.replyTo.equals("")) {
			sender = transport.createSender(message.replyTo);
		} else {
			log.trace("NO REPLY TO REQUIRED");
		}

		Session session = null;
		try {
			boolean hasTPCONV = (message.flags & Connection.TPCONV) == Connection.TPCONV;
			if (hasTPCONV) {
				session = new Session(transport, message.cd, sender);
				log.debug("Created the session");
				int olen = 4;
				Buffer odata = new Buffer("X_OCTET", null);
				odata.setData("ACK".getBytes());
				long result = session.tpsend(odata, olen, 0);
				if (result == -1) {
					session.close();
					return;
				} else {
					session.setCreatedState(message.flags);
				}
			} else {
				log.debug("cd not being set");
			}

			// THIS IS THE FIRST CALL
			Buffer buffer = new Buffer(message.type, message.subtype);
			buffer.setData(message.data);
			// TODO NO SESSIONS
			// NOT PASSING OVER THE SERVICE NAME
			TPSVCINFO tpsvcinfo = new TPSVCINFO(message.serviceName, buffer,
					message.flags, session);
			log.debug("Prepared the data for passing to the service");

			boolean hasTx = (message.control != null && message.control
					.length() != 0);

			log.debug("hasTx=" + hasTx + " ior: " + message.control);

			if (hasTx) // make sure any foreign tx is resumed before calling the
				// service routine
				JtsTransactionImple.resume(message.control);

			log.debug("Invoking the XATMI service");
			Response response = null;
			try {
				response = tpservice(tpsvcinfo);
				log.debug("Service invoked");
				if (!hasTPNOREPLY && response == null) {
					log.error("Error, expected response but none returned");
					response = new Response(Connection.TPFAIL,
							Connection.TPESVCERR, null, 0, 0);
				}
			} catch (Throwable t) {
				log.error("Service error detected", t);
				response = new Response(Connection.TPFAIL,
						Connection.TPESVCERR, null, 0, 0);
			}

			if (hasTx) // and suspend it again
				JtsTransactionImple.suspend();

			if (sender != null && response != null) {
				log.trace("Sending response");
				short rval = response.getRval();
				if (rval != Connection.TPSUCCESS && rval != Connection.TPFAIL) {
					rval = Connection.TPFAIL;
					// TODO SET ROLLBACK ONLY
				}
				Buffer toSend = response.getBuffer();
				int len = response.getLen();
				String type = null;
				String subtype = null;
				byte[] data = null;
				if (toSend != null) {
					toSend.serialize();
					data = toSend.getData();
					type = toSend.getType();
					subtype = toSend.getSubtype();
					if (!type.equals("X_OCTET")) {
						len = toSend.getLength();
					}
				} else {
					type = "X_OCTET";
				}
				sender.send("", rval, response.getRcode(), data, len, response
						.getFlags(), 0, type, subtype);

			} else if (sender == null && response != null) {
				log.error("No sender avaible but message to be sent");
			} else if (sender != null && response == null) {
				log.error("Sender waiting but no response");
			} else {
				log.debug("No need to send a response");
			}
		} finally {
			if (session != null) {
				session.close();
			} else if (sender != null) {
				sender.close();
			}
		}
	}
}
