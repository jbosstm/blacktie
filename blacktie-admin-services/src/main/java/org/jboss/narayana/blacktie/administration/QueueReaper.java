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
package org.jboss.narayana.blacktie.administration;

import java.io.IOException;
import java.util.Properties;

import javax.management.AttributeNotFoundException;
import javax.management.InstanceNotFoundException;
import javax.management.MBeanException;
import javax.management.MBeanServer;
import javax.management.MalformedObjectNameException;
import javax.management.ObjectInstance;
import javax.management.ObjectName;
import javax.management.ReflectionException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.narayana.blacktie.administration.core.AdministrationProxy;
import org.jboss.narayana.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.narayana.blacktie.jatmibroker.core.conf.XMLParser;

public class QueueReaper implements Runnable {
    /** logger */
    private static final Logger log = LogManager.getLogger(QueueReaper.class);

    /** Interval at which to run */
    private long interval = -1; // TODO make this configurable

    /** Thread to run */
    private Thread thread;

    /** Whether the thread is executing */
    private boolean run;

    private Properties prop;

    private MBeanServer beanServerConnection;

    public QueueReaper(AdministrationProxy administrationProxy) throws ConfigurationException {
        this.thread = new Thread(this);
        this.thread.setDaemon(true);
        this.thread.setPriority(Thread.MIN_PRIORITY);
        beanServerConnection = java.lang.management.ManagementFactory.getPlatformMBeanServer();

        prop = new Properties();
        XMLParser.loadProperties("btconfig.xsd", "btconfig.xml", prop);

        this.interval = Integer.parseInt(prop.getProperty("QueueReaperInterval", "30")) * 1000;
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
                ObjectName objName = new ObjectName("jboss.as:subsystem=messaging,hornetq-server=default,jms-queue=*");
                ObjectInstance[] dests = beanServerConnection.queryMBeans(objName, null).toArray(new ObjectInstance[] {});

                for (int i = 0; i < dests.length; i++) {
                    String serviceComponentOfObjectName = dests[i].getObjectName().getCanonicalName();
                    serviceComponentOfObjectName = serviceComponentOfObjectName.substring(
                            serviceComponentOfObjectName.indexOf('_') + 1,
                            serviceComponentOfObjectName.indexOf(",", serviceComponentOfObjectName.indexOf('_')));
                    log.trace("Service name component of ObjectName is: " + serviceComponentOfObjectName);
                    String server = (String) prop.get("blacktie." + serviceComponentOfObjectName + ".server");
                    log.trace("Checking for: " + serviceComponentOfObjectName + " " + server + " "
                            + prop.get("blacktie." + serviceComponentOfObjectName + ".externally-managed-destination"));

                    if ((serviceComponentOfObjectName.startsWith(".") || ((server != null && !(Boolean) prop.get("blacktie."
                            + serviceComponentOfObjectName + ".externally-managed-destination"))))
                            && consumerCount(serviceComponentOfObjectName) == 0) {
                        log.warn("undeploy service pending for " + serviceComponentOfObjectName
                                + " as consumer count is 0, will check again in 30 seconds");
                        long queueReapCheck = System.currentTimeMillis();
                        Thread.sleep(this.interval);

                        // double check consumer is 0
                        if (BlacktieStompAdministrationService.isOlderThanReapCheck(serviceComponentOfObjectName,
                                queueReapCheck) && consumerCount(serviceComponentOfObjectName) == 0) {
                            BlacktieStompAdministrationService.undeployQueue(serviceComponentOfObjectName);
                            log.warn("undeploy service " + serviceComponentOfObjectName + " for consumer is 0");
                        } else {
                            log.debug("Undeploy not required for: " + serviceComponentOfObjectName + " at: " + server
                                    + " as client connected");
                        }
                    } else {
                        log.debug("Undeploy not required for: " + serviceComponentOfObjectName + " at: " + server);
                    }
                }
                log.debug("Sleeping for " + this.interval + " ms");
                Thread.sleep(this.interval);
            } catch (InterruptedException e) {
                log.debug("Sleeping interrupted");
                this.run = false;
            } catch (Exception e) {
                log.error("run ping thread failed with (will wait for: " + interval + " seconds): " + e, e);
                try {
                    Thread.sleep(this.interval);
                } catch (InterruptedException e2) {
                    log.debug("Sleeping interrupted");
                    this.run = false;
                }
            }
        }
    }

    int consumerCount(String serviceName) throws MalformedObjectNameException, NullPointerException,
            AttributeNotFoundException, InstanceNotFoundException, ReflectionException, IOException, MBeanException {
        log.trace(serviceName);
        boolean conversational = false;
        String type = "queue";
        if (!serviceName.startsWith(".")) {
            conversational = (Boolean) prop.get("blacktie." + serviceName + ".conversational");
            type = (String) prop.get("blacktie." + serviceName + ".type");
        }
        String prefix = null;
        if (conversational) {
            prefix = "BTC_";
        } else {
            prefix = "BTR_";
        }

        ObjectName objName = new ObjectName("jboss.as:subsystem=messaging,hornetq-server=default,jms-" + type + "=" + prefix
                + serviceName);
        try {
            Integer count = null;
            if (type.equals("queue")) {
                count = (Integer) beanServerConnection.getAttribute(objName, "consumerCount");
            } else {
                count = (Integer) beanServerConnection.getAttribute(objName, "subscriptionCount");
            }
            return count.intValue();
        } catch (javax.management.InstanceNotFoundException e) {
            log.debug("Instance not found: " + objName);
            return -1;
        }
    }
}
