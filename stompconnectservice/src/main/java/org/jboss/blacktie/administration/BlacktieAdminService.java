package org.jboss.blacktie.administration;

import java.io.StringReader;
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
import org.jboss.blacktie.jatmibroker.core.server.AtmiBrokerServer;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionFactory;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;

import javax.jms.Destination;
import javax.jms.Queue;
import javax.management.Attribute;
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;

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
	private Connection  connection;
	private QueueReaper reaper;

	public void start() throws Exception {
		XMLEnvHandler handler = new XMLEnvHandler("", prop);
		XMLParser xmlenv = new XMLParser(handler, "Environment.xsd");
		xmlenv.parse("Environment.xml");
		JMXServiceURL u = new JMXServiceURL(
				(String)prop.get("JMXURL"));
		JMXConnector c = JMXConnectorFactory.connect(u);
		beanServerConnection = c.getMBeanServerConnection();

		ConnectionFactory connectionFactory = ConnectionFactory.getConnectionFactory();
		connection = connectionFactory.getConnection();
		reaper = new QueueReaper(beanServerConnection);
		reaper.startThread();
		
		log.info("Admin Server Started");
	}

	public void stop() throws Exception {
		connection.close();
		reaper.stopThread();
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
	
	private Element stringToElement(String s) throws Exception {
		StringReader sreader = new StringReader(s.trim());
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		DocumentBuilder parser = factory.newDocumentBuilder();
		Document doc = parser.parse(new InputSource(sreader));
		return doc.getDocumentElement();
	}
	
	private Response callAdminService(String serverName, int id, String command) throws ConnectionException {
		int sendlen = command.length() + 1;
		Buffer sendbuf = connection.tpalloc("X_OCTET", null);
		sendbuf.setData(command.getBytes());
		
		String service = serverName + "_ADMIN_" + id;

		Response rcvbuf = connection.tpcall(service, sendbuf, sendlen, 0);
		return rcvbuf;
	}

	public Element listServiceStatus(String serverName, int id) {
		String command = "status";
		Response buf = null;
		String status = null;
		try {
			buf = callAdminService(serverName, id, command);
			if(buf != null) {
				byte[] received = buf.getBuffer().getData();
				if(received[0] == '1') {				
					status = new String(received, 1, received.length - 1);
					log.info("status is " + status);
					return stringToElement(status);
				}
			}
		} catch (ConnectionException e) {
			log.error("call server " + serverName + " id " + id + " failed with " + e.getTperrno());
		} catch (Exception e) {
			log.error("response " + status + " error with " + e);
		}
		return null;
	}

	public Boolean advertise(String serverName, int id, String serviceName) {
		String command = "advertise," + serviceName + ",";
		try {
			Response buf = callAdminService(serverName, id, command);
			if(buf != null) {
				byte[] received = buf.getBuffer().getData();
				return (received[0] == '1');
			}
		} catch (ConnectionException e) {
			log.error("call server " + serverName + " id " + id + " failed with " + e.getTperrno());
		}
		return false;
	}

	public Boolean unadvertise(String serverName, int id, String serviceName) {
		String command = "unadvertise," + serviceName + ",";
		try {
			Response buf = callAdminService(serverName, id, command);
			if(buf != null) {
				byte[] received = buf.getBuffer().getData();
				return (received[0] == '1');
			}
		} catch (ConnectionException e) {
			log.error("call server " + serverName + " id " + id + " failed with " + e.getTperrno());
		}
		return false;
	}

	public void shutdown(String serverName, int id) {
		String command = "serverdone";
		try {
			callAdminService(serverName, id, command);
		} catch (ConnectionException e) {
			log.error("call server " + serverName + " id " + id + " failed with " + e.getTperrno());
			e.printStackTrace();
		}
	}
}
