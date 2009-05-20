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
package org.jboss.blacktie.jatmibroker.server;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.transport.Message;
import org.jboss.blacktie.jatmibroker.transport.Receiver;
import org.jboss.blacktie.jatmibroker.transport.Sender;
import org.jboss.blacktie.jatmibroker.transport.Transport;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class ServiceDispatcher extends Thread {
	private static final Logger log = LogManager
			.getLogger(ServiceDispatcher.class);
	private BlacktieService callback;
	private String serviceName;

	private Receiver receiver;
	private Sender endpointQueue;
	private Transport connection;

	ServiceDispatcher(Transport connection, String serviceName,
			BlacktieService callback, Receiver receiver)
			throws InstantiationException, IllegalAccessException {
		this.serviceName = serviceName;
		this.callback = callback;
		this.receiver = receiver;
		this.connection = connection;
		start();
	}

	public void run() {
		while (true) {
			Message message = receiver.receive(0);
			try {
				endpointQueue = connection.createSender(message.replyTo);

				// TODO HANDLE CONTROL
				// THIS IS THE FIRST CALL
				TPSVCINFO tpsvcinfo = new TPSVCINFO(serviceName, message.data,
						message.len, message.flags, -1);

				Response response = callback.tpservice(tpsvcinfo);
				// TODO THIS SHOULD INVOKE THE CLIENT HANDLER
				// odata.value = serviceRequest.getBytes();
				// olen.value = serviceRequest.getLength();
				endpointQueue.send("", response.getRval(), response.getRcode(),
						response.getData(), response.getLen(), response
								.getFlags(), 0);
			} catch (Throwable t) {
				log.error("Could not service the request");
			}
		}
	}
}
