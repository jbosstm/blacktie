package org.jboss.blacktie.administration;

import java.util.Properties;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;

import java.util.List;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.conf.XMLEnvHandler;
import org.jboss.blacktie.jatmibroker.core.conf.XMLParser;

import javax.jms.Destination;
import javax.jms.Queue;
import javax.management.Attribute;
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

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

public class BlacktieAdminService implements BlacktieAdminServiceMBean {
	private static final Logger log = LogManager.getLogger(BlacktieAdminService.class);
	private Properties prop = new Properties();
	private MBeanServerConnection beanServerConnection;

	public void start() throws Exception {
		JMXServiceURL u = new JMXServiceURL(
				"service:jmx:rmi:///jndi/rmi://localhost:1090/jmxconnector");
		JMXConnector c = JMXConnectorFactory.connect(u);
		beanServerConnection = c.getMBeanServerConnection();

		XMLEnvHandler handler = new XMLEnvHandler("", prop);
		XMLParser xmlenv = new XMLParser(handler, "Environment.xsd");
		xmlenv.parse("Environment.xml");
		log.info("Admin Server Started");
	}

	public void stop() throws Exception {
		log.info("Admin Server Stopped");
	}

	public String getDomainName() {
		return prop.getProperty("blacktie.domain.name");
	}

	public List getServersName() throws Exception {
		ArrayList<String> serversName = new ArrayList<String>();
		ObjectName objName = new ObjectName(
				"jboss.messaging:service=ServerPeer");
		HashSet dests = (HashSet) beanServerConnection.getAttribute(objName, "Destinations");

		Iterator<Destination> it = dests.iterator();
		while (it.hasNext()) {
			Destination dest = it.next();
			if (dest instanceof Queue) {
				String qname = ((Queue) dest).getQueueName();
				if (qname.indexOf("ADMIN") > 0) {
					log.debug("find server " + qname);
					serversName.add(qname);
				}
			}
		}
		return serversName;
	}
}
