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
import org.jboss.blacktie.jatmibroker.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.conf.AtmiBrokerClientXML;
import org.jboss.blacktie.jatmibroker.transport.Message;
import org.jboss.blacktie.jatmibroker.transport.Sender;
import org.jboss.blacktie.jatmibroker.transport.Transport;
import org.jboss.blacktie.jatmibroker.transport.TransportFactory;

public abstract class Service implements BlacktieService {
	private static final Logger log = LogManager.getLogger(Service.class);
	private Transport transport;

	private synchronized Transport getTransport() throws JAtmiBrokerException {
		if (transport == null) {
			Properties properties = null;
			AtmiBrokerClientXML xml = new AtmiBrokerClientXML();
			try {
				properties = xml.getProperties();
			} catch (Exception e) {
				throw new JAtmiBrokerException("Could not load properties", e);
			}
			transport = TransportFactory.loadTransportFactory(properties)
					.createTransport();
		}
		return transport;
	}

	protected void processMessage(Message message) throws JAtmiBrokerException,
			ConnectionException {
		Transport transport = getTransport();
		Sender sender = transport.createSender(message.replyTo);
		Session session = new Session(transport, message.cd, sender, null);

		// TODO HANDLE CONTROL
		// THIS IS THE FIRST CALL
		Buffer buffer = new Buffer(null, null);
		buffer.setData(message.data);
		// TODO NO SESSIONS
		// NOT PASSING OVER THE SERVICE NAME
		TPSVCINFO tpsvcinfo = new TPSVCINFO(null, buffer, message.flags,
				session);

		Response response = tpservice(tpsvcinfo);
		// TODO THIS SHOULD INVOKE THE CLIENT HANDLER
		// odata.value = serviceRequest.getBytes();
		// olen.value = serviceRequest.getLength();
		sender.send("", response.getRval(), response.getRcode(), response
				.getBuffer().getData(), response.getLen(), response.getFlags(),
				0);
	}
}
