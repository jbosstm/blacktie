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
package org.jboss.blacktie.jatmibroker.core.server;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.Message;
import org.jboss.blacktie.jatmibroker.core.transport.Receiver;
import org.jboss.blacktie.jatmibroker.xatmi.BlackTieService;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

/**
 * This is the compatriot to the MDBBlacktieService found in the xatmi.mdb
 * package. It is a wrapper for user services.
 */
public class ServiceDispatcher extends BlackTieService implements Runnable {
	private static final Logger log = LogManager
			.getLogger(ServiceDispatcher.class);
	private Service callback;
	private Receiver receiver;
	private Thread thread;
	private volatile boolean closed;

	ServiceDispatcher(String serviceName, Service callback, Receiver receiver) {
		super(serviceName);
		this.callback = callback;
		this.receiver = receiver;
		thread = new Thread(this, serviceName + "-Dispatcher");
		thread.start();
		log.debug("Created: " + thread.getName());
	}

	public void run() {
		log.debug("Running");

		while (!closed) {
			Message message = null;
			try {
				message = receiver.receive(0);
				log.trace("Received");
			} catch (ConnectionException e) {
				if (closed) {
					break;
				}
				if (e.getTperrno() == Connection.TPETIME) {
					log.debug("Got a timeout");
				} else {
					log.error("Could not receive the message: "
							+ e.getMessage(), e);
					break;
				}
			}

			if (message != null) {
				try {

					this.processMessage(message);
					log.trace("Processed");
				} catch (Throwable t) {
					log.error("Can't process the message", t);
				}
			}
		}
	}

	public void startClose() {
		closed = true;
		log.trace("Closed set");
	}

	public void close() throws ConnectionException {
		log.trace("closing");

		log.trace("Interrupting");
		thread.interrupt();
		log.trace("Interrupted");

		try {
			log.trace("Joining");
			thread.join();
			log.trace("Joined");
		} catch (InterruptedException e) {
			log.error("Could not join the dispatcher", e);
		}
		log.trace("closed");
	}

	public Response tpservice(TPSVCINFO svcinfo) throws ConnectionException,
			ConfigurationException {
		log.trace("Invoking callback");
		return callback.tpservice(svcinfo);
	}
}
