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
import org.jboss.blacktie.jatmibroker.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.transport.Message;
import org.jboss.blacktie.jatmibroker.transport.Receiver;
import org.jboss.blacktie.jatmibroker.transport.Transport;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class ServiceDispatcher extends Service implements Runnable {
	private static final Logger log = LogManager
			.getLogger(ServiceDispatcher.class);
	private BlacktieService callback;
	private Receiver receiver;
	private Thread thread;

	ServiceDispatcher(Transport transport, String serviceName,
			BlacktieService callback, Receiver receiver) {
		super();
		this.callback = callback;
		this.receiver = receiver;
		thread = new Thread(this);
		thread.start();
		log.debug("Created");
	}

	public void run() {
		log.debug("Running");
		while (true) {
			Message message = receiver.receive(0);
			log.trace("Recieved");
			try {
				this.processMessage(message);
			} catch (Throwable t) {
				log.error("Could not service the request", t);
			}
		}
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		log.trace("Invoking callback");
		return callback.tpservice(svcinfo);
	}
}
