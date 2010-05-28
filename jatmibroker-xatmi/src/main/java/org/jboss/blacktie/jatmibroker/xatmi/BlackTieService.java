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

import javax.naming.NamingException;
import javax.transaction.InvalidTransactionException;
import javax.transaction.SystemException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.JtsTransactionImple;
import org.jboss.blacktie.jatmibroker.core.transport.Message;
import org.jboss.blacktie.jatmibroker.jab.JABException;
import org.jboss.blacktie.jatmibroker.jab.JABTransaction;

/**
 * MDB services implementations extend this class as it provides the core
 * service template method. For non MDB services on the Service interface need
 * be implemented.
 */
public abstract class BlackTieService implements Service {
	/**
	 * The logger to use.
	 */
	private static final Logger log = LogManager
			.getLogger(BlackTieService.class);

	/**
	 * The name of the service.
	 */
	private String name;

	/**
	 * The service needs the name of the service so that it can be resolved in
	 * the btconfig.xml file
	 * 
	 * @param name
	 *            The name of the service
	 */
	public BlackTieService(String name) {
		this.name = name;
		log.debug("Service created: " + name);
	}

	/**
	 * Entry points should pass control to this method as soon as reasonably
	 * possible.
	 * 
	 * @param message
	 *            The message to process
	 * @throws ConnectionException
	 * @throws ConnectionException
	 *             In case communication fails
	 * @throws ConfigurationException
	 * @throws NamingException
	 * @throws JABException
	 * @throws SystemException
	 * @throws IllegalStateException
	 * @throws InvalidTransactionException
	 */
	protected void processMessage(Message message) throws ConnectionException,
			ConfigurationException, NamingException,
			InvalidTransactionException, IllegalStateException,
			SystemException, JABException {
		Connection connection = ConnectionFactory.getConnectionFactory()
				.getConnection();
		try {
			log.trace("Service invoked");
			boolean hasTPNOREPLY = (message.flags & Connection.TPNOREPLY) == Connection.TPNOREPLY;

			Session serviceSession = connection.createServiceSession(name,
					message.cd, message.replyTo);
			boolean hasTPCONV = (message.flags & Connection.TPCONV) == Connection.TPCONV;
			boolean isConversational = ((Boolean) connection.properties
					.get("blacktie." + name + ".conversational")) == true;
			if (hasTPCONV && isConversational) {
				int olen = 4;
				X_OCTET odata = new X_OCTET(olen);
				odata.setByteArray("ACK".getBytes());
				long result = serviceSession.tpsend(odata, 0);
				if (result == -1) {
					log.error("Could not send ack");
					serviceSession.close();
					return;
				} else {
					log.debug("Sent ack");
					serviceSession.setCreatedState(message.flags);
				}
			} else if (!hasTPCONV && !isConversational) {
				log.debug("Session was not a TPCONV");
			} else {
				log.error("Session was invoked in an improper manner");
				// Even though we can provide the cd we don't as
				// atmibroker-xatmi doesn't because tpreturn doesn't
				serviceSession.getSender().send("", Connection.TPFAIL,
						Connection.TPESVCERR, null, 0, 0, 0, 0, null, null);
				log.error("Error reported");
				return;
			}
			log.debug("Created the session");
			// To respond with
			boolean hasTx = false;
			boolean responseSendable = !hasTPNOREPLY;
			short rval = Connection.TPFAIL;
			int rcode = Connection.TPESVCERR;
			byte[] data = null;
			int len = 0;
			int flags = 0;
			String type = null;
			String subtype = null;
			try {
				// THIS IS THE FIRST CALL
				Buffer buffer = null;
				if (message.type != null && !message.type.equals("")) {
					buffer = connection.tpalloc(message.type, message.subtype,
							message.len);
					buffer.deserialize(message.data);
				}
				TPSVCINFO tpsvcinfo = new TPSVCINFO(message.serviceName,
						buffer, message.flags, (hasTPCONV ? serviceSession
								: null), connection, message.len);
				log.debug("Prepared the data for passing to the service");

				hasTx = (message.control != null && message.control.length() != 0);

				log.debug("hasTx=" + hasTx + " ior: " + message.control);

				if (hasTx) // make sure any foreign tx is resumed before calling
					// the
					// service routine
					JtsTransactionImple.resume(message.control);

				log.debug("Invoking the XATMI service");
				Response response = null;
				try {
					response = tpservice(tpsvcinfo);
					log.debug("Service invoked");
					if (!hasTPNOREPLY && response == null) {
						log.error("Error, expected response but none returned");
					}
				} catch (Throwable t) {
					log.error("Service error detected", t);
				}

				if (!hasTPNOREPLY && serviceSession.getSender() != null) {
					log.trace("Sending response");
					if (response != null) {
						rval = response.getRval();
						rcode = response.getRcode();
						if (rval != Connection.TPSUCCESS
								&& rval != Connection.TPFAIL) {
							rval = Connection.TPFAIL;
						}
					}
					if (connection.hasOpenSessions()) {
						rcode = Connection.TPESVCERR;
						rval = Connection.TPFAIL;
					}
					if (rval == Connection.TPFAIL) {
						if (JABTransaction.current() != null) {
							try {
								JABTransaction.current().rollback_only();
							} catch (JABException e) {
								throw new ConnectionException(
										Connection.TPESYSTEM,
										"Could not mark transaction for rollback only");
							}
						}
					}

					if (response != null) {
						Buffer toSend = response.getBuffer();
						if (toSend != null) {
							len = toSend.getLen();
							data = toSend.serialize();
							type = toSend.getType();
							subtype = toSend.getSubtype();
							if (!type.equals("X_OCTET")) {
								len = data.length;
							}
						}
						flags = response.getFlags();
					}
					log.debug("Will return desired message");
				} else if (!hasTPNOREPLY && serviceSession.getSender() == null) {
					log.error("No sender avaible but message to be sent");
					responseSendable = false;
				} else {
					log.debug("No need to send a response");
				}
			} finally {
				if (hasTx) // and suspend it again
					JtsTransactionImple.suspend();
				if (responseSendable) {
					// Even though we can provide the cd we don't as
					// atmibroker-xatmi doesn't because tpreturn doesn't
					serviceSession.getSender().send("", rval, rcode, data, len,
							0, flags, 0, type, subtype);
				}
			}
		} finally {
			connection.close();
		}
	}
}
