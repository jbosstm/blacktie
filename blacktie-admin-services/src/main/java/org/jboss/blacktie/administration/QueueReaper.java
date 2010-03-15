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
package org.jboss.blacktie.administration;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Properties;

import javax.jms.Destination;
import javax.jms.Queue;
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.XMLEnvHandler;
import org.jboss.blacktie.jatmibroker.core.conf.XMLParser;

public class QueueReaper implements Runnable {
	/** logger */
	private static final Logger log = LogManager.getLogger(QueueReaper.class);

	/** Interval at which to run */
	private long interval = 30 * 1000; // TODO make this configurable

	/** Thread to run */
	private Thread thread;

	/** Whether the thread is executing */
	private boolean run;

	private MBeanServerConnection beanServerConnection;

	public QueueReaper(MBeanServerConnection conn) {
		this.thread = new Thread(this);
		this.thread.setDaemon(true);
		this.thread.setPriority(Thread.MIN_PRIORITY);
		this.beanServerConnection = conn;
	}

	public void startThread() {
		if (!this.run) {
			this.run = true;
			if (this.thread.isInterrupted()) {
				Thread.interrupted();
			}
			this.thread.start();
		} else {
			log.debug("Thread already running");
		}
	}

	public void stopThread() {
		if (!this.thread.isInterrupted()) {
			this.thread.interrupt();
		}
		this.run = false;
		try {
			this.thread.join();
		} catch (InterruptedException e) {
			log.warn("Could not join with reaper: " + e.getMessage());
		}
	}

	public boolean isRunning() {
		return this.run;
	}

	public void run() {
		while (this.run) {
			try {
				ObjectName objName = new ObjectName(
						"jboss.messaging:service=ServerPeer");
				HashSet<Destination> dests = (HashSet<Destination>) beanServerConnection
						.getAttribute(objName, "Destinations");

				Properties prop = new Properties();
				XMLEnvHandler handler = new XMLEnvHandler(prop);
				XMLParser xmlenv = new XMLParser(handler, "Environment.xsd");
				xmlenv.parse("Environment.xml", true);

				Iterator<Destination> it = dests.iterator();
				while (it.hasNext()) {
					Destination dest = it.next();
					if (dest instanceof Queue) {
						String serviceName = ((Queue) dest).getQueueName();
						String server = (String) prop.get("blacktie."
								+ serviceName + ".server");
						long queueReapCheck = System.currentTimeMillis();
						if ((server != null || serviceName.contains("_ADMIN_"))
								&& isCreatedProgrammatically(serviceName)
								&& consumerCount(serviceName) == 0) {
							log
									.warn("undeploy service pending for "
											+ serviceName
											+ " as consumer count is 0, will check again in 30 seconds");

							this.interval = Integer.parseInt(prop.getProperty(
									"QueueReaperInterval", "30")) * 1000;
							Thread.sleep(this.interval);

							// double check consumer is 0
							if (isOlderThanReapCheck(serviceName,
									queueReapCheck)
									&& consumerCount(serviceName) == 0) {
								undeployQueue(serviceName);
								log.warn("undeploy service " + serviceName
										+ " for consumer is 0");
							} else {
								log.info("undeploy service not required for "
										+ serviceName);
							}
						} else {
							log.debug("Could not determine the server for: "
									+ serviceName + " at: " + server);
						}
					}
				}
				log.debug("Sleeping for " + this.interval + " ms");
				Thread.sleep(this.interval);
			} catch (InterruptedException e) {
				log.debug("Sleeping interrupted");
				this.run = false;
			} catch (Exception e) {
				log.error("run ping thread failed with " + e);
				this.run = false;
			}
		}
	}

	int consumerCount(String serviceName) throws Exception {
		ObjectName objName = new ObjectName(
				"jboss.messaging.destination:service=Queue,name=" + serviceName);
		Integer count = (Integer) beanServerConnection.getAttribute(objName,
				"ConsumerCount");
		return count.intValue();
	}

	Boolean isCreatedProgrammatically(String serviceName) throws Exception {
		ObjectName objName = new ObjectName(
				"jboss.messaging.destination:service=Queue,name=" + serviceName);
		return (Boolean) beanServerConnection.getAttribute(objName,
				"CreatedProgrammatically");
	}

	private boolean isOlderThanReapCheck(String serviceName, long queueReapCheck) {
		// TODO THIS WILL NOT CLUSTER AS IT ASSUMES THE QUEUE WAS CREATED BY
		// THIS SERVER
		synchronized (BlacktieStompAdministrationService.QUEUE_CREATION_TIMES) {
			boolean toReturn = false;
			Long creationTime = BlacktieStompAdministrationService.QUEUE_CREATION_TIMES
					.get(serviceName);
			if (creationTime != null) {
				toReturn = creationTime < queueReapCheck;
				if (!toReturn) {
					log.warn("New queue will be ignored: " + serviceName);
				}
			}
			return toReturn;
		}
	}

	int undeployQueue(String serviceName) {
		int result = 0;

		try {
			ObjectName objName = new ObjectName(
					"jboss.messaging:service=ServerPeer");
			beanServerConnection.invoke(objName, "undeployQueue",
					new Object[] { serviceName },
					new String[] { "java.lang.String" });
			result = 1;
		} catch (Throwable t) {
			log.error("Could not undeploy queue of " + serviceName, t);
		}

		return result;
	}
}
