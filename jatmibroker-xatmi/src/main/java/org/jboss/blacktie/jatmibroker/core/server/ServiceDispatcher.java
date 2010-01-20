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
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class ServiceDispatcher extends Service implements Runnable {
	private static final Logger log = LogManager
			.getLogger(ServiceDispatcher.class);
	private BlacktieService callback;
	private Receiver receiver;
	private Thread thread;
	private volatile boolean closed;
	private Object closer = new Object();
	private Object dier = new Object();
	private boolean dead;

	ServiceDispatcher(String serviceName, BlacktieService callback,
			Receiver receiver) throws ConfigurationException,
			ConnectionException {
		super(serviceName);
		this.callback = callback;
		this.receiver = receiver;
		thread = new Thread(this, serviceName + "-Dispatcher");
		thread.start();
		log.debug("Created");
	}

	public void run() {
		log.debug("Running");
		try {
			while (!closed) {
				try {
					Message message = receiver.receive(0);
					log.trace("Received");
					try {
						this.processMessage(message);
						log.trace("Processed");
					} catch (Throwable t) {
						log.error("Can't process the message", t);
					}

				} catch (ConnectionException e) {
					if (closed) {
						throw e;
					}
					if (e.getTperrno() == Connection.TPETIME) {
						log.debug("Got a timeout");
					} else {
						throw e;
					}
				}
			}
		} catch (Throwable t) {
			if (!closed) {
				log.error("Could not receive the message", t);
			} else {
				log.debug("Did not receive the message during shutdown: " + t.getMessage(), t);
			}
		}

		synchronized (closer) {
			synchronized (dier) {
				if (!closed) {
					try {
						log.trace("Waiting dier");
						dier.wait();
						log.trace("Waited dier");
					} catch (InterruptedException e) {
						log.warn("Could not wait");
					}
				} else {
					log.trace("Not waiting dier");
				}
			}
			log.trace("Notifying closer");
			closer.notify();
			log.trace("Notified closer");
			dead = true;
		}
	}

	public void startClose() {
		closed = true;
		log.trace("Closed set");
	}

	public void close() throws ConnectionException {
		synchronized (dier) {
			log.trace("Notifying dier");
			dier.notify();
			log.trace("Notified dier");
		}
		synchronized (closer) {
			if (!dead) {
				try {
					log.trace("Waiting closer");
					closer.wait();
					log.trace("Waited closer");
				} catch (InterruptedException e) {
					log.error("Could not wait for the responder", e);
				}
			}
		}
		super.close();
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		log.trace("Invoking callback");
		return callback.tpservice(svcinfo);
	}
}
