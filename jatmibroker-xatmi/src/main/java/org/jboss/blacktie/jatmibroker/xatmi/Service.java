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
	 * Entry points should pass control to this method as soon as reasonably
	 * possible.
	 * 
	 * @param message
	 *            The message to process
	 * @throws ConnectionException
	 *             In case communication fails
	 */
	protected void processMessage(Message message) throws ConnectionException {
		if (JtsTransactionImple.hasTransaction()) {
			log
					.error("Blacktie MDBs must not be called with a transactional context");
		} else {
			log.trace("Service invoked");
		}
		log.trace("obtained transport");
		Sender sender = null;
		if (message.replyTo != null) {
			sender = transport.createSender(message.replyTo);
		} else {
			log.trace("NO REPLY TO REQUIRED");
		}
		Session session = new Session(transport, message.cd, sender);
		log.debug("Created the session");

		// THIS IS THE FIRST CALL
		Buffer buffer = new Buffer(null, null);
		buffer.setData(message.data);
		// TODO NO SESSIONS
		// NOT PASSING OVER THE SERVICE NAME
		TPSVCINFO tpsvcinfo = new TPSVCINFO(null, buffer, message.flags,
				session);
		log.debug("Prepared the data for passing to the service");

		boolean hasTx = (message.control != null && message.control.length() != 0);

		log.debug("hasTx=" + hasTx + " ior: " + message.control);

		try {
			if (hasTx) // make sure any foreign tx is resumed before calling the
				// service routine
				JtsTransactionImple.resume(message.control);

			log.debug("Invoking the XATMI service");
			Response response = tpservice(tpsvcinfo);
			log.debug("Service invoked");

			if (hasTx) // and suspend it again
				JtsTransactionImple.suspend();

			if (sender != null && response != null) {
				log.trace("Sending response");
				// TODO
				// odata.value = serviceRequest.getBytes();
				// olen.value = serviceRequest.getLength();
				sender.send(null, response.getRval(), response.getRcode(),
						response.getBuffer().getData(), response.getLen(),
						response.getFlags(), 0);

			} else if (sender == null && response != null) {
				log.error("No sender avaible but message to be sent");
			} else if (sender != null && response == null) {
				log.error("Sender waiting but no response");
			} else {
				log.debug("No need to send a response");
			}
		} finally {
			session.close();
		}
	}
}
