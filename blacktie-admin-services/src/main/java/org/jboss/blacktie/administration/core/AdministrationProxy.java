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

package org.jboss.blacktie.administration.core;

import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Properties;
import java.util.Set;

import javax.jms.Destination;
import javax.jms.Queue;
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.conf.XMLEnvHandler;
import org.jboss.blacktie.jatmibroker.core.conf.XMLParser;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionFactory;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;

public class AdministrationProxy {
	private static final Logger log = LogManager
			.getLogger(AdministrationProxy.class);
	private Properties prop = new Properties();
	private JMXConnector c;
	private MBeanServerConnection beanServerConnection;
	private Connection connection;
	private Set<String> servers;

	public AdministrationProxy() throws IOException, ConfigurationException,
			ConnectionException {
		log.info("debug Administration Proxy");
		XMLEnvHandler handler = new XMLEnvHandler("", prop);
		XMLParser xmlenv = new XMLParser(handler, "Environment.xsd");
		xmlenv.parse("Environment.xml", true);
		servers = (Set<String>) prop.get("blacktie.domain.servers");
		ConnectionFactory cf = ConnectionFactory.getConnectionFactory();
		connection = cf.getConnection();
		JMXServiceURL u = new JMXServiceURL((String) prop.get("JMXURL"));
		c = JMXConnectorFactory.connect(u);
		beanServerConnection = c.getMBeanServerConnection();
		log.info("Created Administration Proxy");
	}

	private Element stringToElement(String s) throws Exception {
		StringReader sreader = new StringReader(s.trim());
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		DocumentBuilder parser = factory.newDocumentBuilder();
		Document doc = parser.parse(new InputSource(sreader));
		return doc.getDocumentElement();
	}

	private String elementToString(Element element) throws Exception {
		// Set up the output transformer
		TransformerFactory transfac = TransformerFactory.newInstance();
		Transformer trans = transfac.newTransformer();
		trans.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
		trans.setOutputProperty(OutputKeys.INDENT, "yes");

		StringWriter sw = new StringWriter();
		StreamResult sr = new StreamResult(sw);
		DOMSource source = new DOMSource(element);
		trans.transform(source, sr);
		return sw.toString();
	}

	private Response callAdminService(String serverName, int id, String command)
			throws ConnectionException {
		log.trace("callAdminService");
		int sendlen = command.length() + 1;
		X_OCTET sendbuf = (X_OCTET) connection.tpalloc("X_OCTET", null);
		sendbuf.setByteArray(command.getBytes());

		String service = serverName + "_ADMIN_" + id;

		Response rcvbuf = connection.tpcall(service, sendbuf, sendlen, 0);
		return rcvbuf;
	}
	
	private Boolean callAdminCommand(String serverName, int id, String command) {
		log.trace("callAdminCommand");
		try {
			Response buf = callAdminService(serverName, id, command);
			if (buf != null) {
				byte[] received = ((X_OCTET) buf.getBuffer()).getByteArray();
				return (received[0] == '1');
			}
		} catch (ConnectionException e) {
			log.error("call server " + serverName + " id " + id + " command " 
					+ command + " failed with " + e.getTperrno());
		}
		return false;
	}

	private Boolean advertise(String serverName, int id, String serviceName) {
		log.trace("advertise");
		String command = "advertise," + serviceName + ",";
		return callAdminCommand(serverName, id, command);
	}

	private Boolean unadvertise(String serverName, int id, String serviceName) {
		log.trace("unadvertise");
		String command = "unadvertise," + serviceName + ",";
		return callAdminCommand(serverName, id, command);
	}

	public String getDomainName() {
		log.trace("getDomainName");
		return prop.getProperty("blacktie.domain.name");
	}

	public String getSoftwareVersion() {
		log.trace("getSoftwareVersion");
		return prop.getProperty("blacktie.domain.version");
	}

	public Boolean pauseDomain() {
		log.trace("pauseDomain");
		Boolean result = true;
		List<String> servers = listRunningServers();

		for (int i = 0; i < servers.size(); i++) {
			result = pauseServer(servers.get(i)) && result;
		}
		return result;
	}
	
	public Boolean pauseServer(String serverName) {
		log.trace("pauseServer");
		Boolean result = true;
		List<Integer> ids = listRunningInstanceIds(serverName);

		for (int i = 0; i < ids.size(); i++) {
			result = pauseServerById(serverName, ids.get(i)) && result;
		}
		return result;
	}
	
	public Boolean pauseServerById(String serverName, int id) {
		log.trace("pauseServerById");
		return callAdminCommand(serverName, id, "pause");
	}

	public Boolean resumeDomain() {
		log.trace("resumeDomain");
		Boolean result = true;
		List<String> servers = listRunningServers();

		for (int i = 0; i < servers.size(); i++) {
			result = resumeServer(servers.get(i)) && result;
		}
		return result;
	}
	
	public Boolean resumeServer(String serverName) {
		log.trace("resumeServer");
		Boolean result = true;
		List<Integer> ids = listRunningInstanceIds(serverName);

		for (int i = 0; i < ids.size(); i++) {
			result = resumeServerById(serverName, ids.get(i)) && result;
		}
		return result;	
	}
	
	public Boolean resumeServerById(String serverName, int id) {
		log.trace("resumeServerById");
		return callAdminCommand(serverName, id, "resume");
	}

	public List<String> getServerList() {
		log.trace("getServerList");
		ArrayList<String> serverList = new ArrayList<String>();

		for (String server : servers) {
			serverList.add(server);
		}
		return serverList;
	}

	@SuppressWarnings("unchecked")
	public List<String> listRunningServers() {
		log.trace("listRunningServers");
		List<String> runningServerList = new ArrayList<String>();

		try {
			ObjectName objName = new ObjectName(
					"jboss.messaging:service=ServerPeer");
			HashSet dests = (HashSet) beanServerConnection.getAttribute(
					objName, "Destinations");

			Iterator<Destination> it = dests.iterator();
			while (it.hasNext()) {
				Destination dest = it.next();
				if (dest instanceof Queue) {
					String qname = ((Queue) dest).getQueueName();
					for (String server : servers) {
						// int index = qname.indexOf(server + "_ADMIN_");
						// log.debug("server is " + server + " qname is " +
						// qname + " index is " + index);
						if (qname.indexOf(server + "_ADMIN_") >= 0
								&& !runningServerList.contains(server)) {
							runningServerList.add(server);
						}
					}
				}
			}
		} catch (Exception e) {
			log.error(e);
		}
		return runningServerList;
	}

	@SuppressWarnings("unchecked")
	public List<Integer> listRunningInstanceIds(String serverName) {
		log.trace("listRunningInstanceIds");
		ArrayList<Integer> ids = new ArrayList<Integer>();

		try {
			ObjectName objName = new ObjectName(
					"jboss.messaging:service=ServerPeer");
			HashSet dests = (HashSet) beanServerConnection.getAttribute(
					objName, "Destinations");

			Iterator<Destination> it = dests.iterator();
			while (it.hasNext()) {
				Destination dest = it.next();
				if (dest instanceof Queue) {
					String qname = ((Queue) dest).getQueueName();
					int index = qname.indexOf(serverName + "_ADMIN_");

					if (index >= 0) {
						ids.add(new Integer(qname.substring(index
								+ serverName.length() + 7)));
					}
				}
			}
		} catch (Exception e) {
			log.error(e);
		}

		return ids;
	}

	public Element getServersStatus() {
		log.trace("getServersStatus");
		try {
			String status = "<servers>";

			for (String server : servers) {
				status += "<server>";
				status += "<name>" + server + "</name>";
				List<Integer> ids = listRunningInstanceIds(server);
				if (ids.size() > 0) {
					status += "<instances>";
					for (int i = 0; i < ids.size(); i++) {
						status += "<instance>";
						status += "<id>" + ids.get(i) + "</id>";
						status += "<status>1</status>";
						status += "</instance>";
					}
					status += "</instances>";
				}
				status += "</server>";
			}

			status += "</servers>";
			return stringToElement(status);
		} catch (Exception e) {
			log.error(e);
			return null;
		}
	}

	public Element listServiceStatus(String serverName, String serviceName) {
		log.trace("listServiceStatus");
		String servers;
		Element status = null;
		List<Integer> ids = listRunningInstanceIds(serverName);

		if (ids.size() == 0) {
			return null;
		}

		try {
			servers = "<servers>";
			for (int i = 0; i < ids.size(); i++) {
				Element result = listServiceStatusById(serverName, ids.get(i),
						serviceName);
				if (result != null) {
					servers += "<instance><id>" + ids.get(i) + "</id>";
					servers += elementToString(result);
					servers += "</instance>";
				}
			}
			servers += "</servers>";
			status = stringToElement(servers);
		} catch (Exception e) {
			log.error(e);
		}

		return status;
	}

	public Boolean advertise(String serverName, String serviceName) {
		log.trace("advertise");
		List<Integer> ids = listRunningInstanceIds(serverName);
		Boolean result = true;

		for (int i = 0; i < ids.size(); i++) {
			result = advertise(serverName, ids.get(i), serviceName) && result;
		}

		return result;
	}

	public Boolean unadvertise(String serverName, String serviceName) {
		log.trace("unadvertise");
		List<Integer> ids = listRunningInstanceIds(serverName);
		Boolean result = true;

		for (int i = 0; i < ids.size(); i++) {
			result = unadvertise(serverName, ids.get(i), serviceName) && result;
		}

		return result;
	}

	public void shutdown(String serverName, int id) {
		log.trace("shutdown");
		String command = "serverdone";
		try {
			if (id == 0) {
				List<Integer> ids = listRunningInstanceIds(serverName);
				for (int i = 0; i < ids.size(); i++) {
					callAdminService(serverName, ids.get(i), command);
				}
			} else {
				callAdminService(serverName, id, command);
			}
		} catch (ConnectionException e) {
			log.error("call server " + serverName + " id " + id
					+ " failed with " + e.getTperrno(), e);
		} catch (RuntimeException e) {
			log.error("Could not shutdown server: " + e.getMessage(), e);
			throw e;
		}
	}

	public long getServiceCounterById(String serverName, int id,
			String serviceName) {
		log.trace("getServiceCounterById");
		long counter = 0;
		String command = "counter," + serviceName + ",";

		try {
			Response buf = callAdminService(serverName, id, command);
			if (buf != null) {
				byte[] received = ((X_OCTET) buf.getBuffer()).getByteArray();
				counter = Long.parseLong(new String(received, 1,
						received.length - 1));
			}
		} catch (ConnectionException e) {
			log.error("call server " + serverName + " id " + id
					+ " failed with " + e.getTperrno());
		}

		return counter;
	}

	public long getServiceCounter(String serverName, String serviceName) {
		log.trace("getServiceCounter");
		long counter = 0;
		List<Integer> ids = listRunningInstanceIds(serverName);

		for (int i = 0; i < ids.size(); i++) {
			counter += getServiceCounterById(serverName, ids.get(i),
					serviceName);
		}

		return counter;
	}

	public Boolean reloadDomain() {
		log.trace("reloadDomain");
		Boolean result = true;
		List<String> servers = listRunningServers();

		for (int i = 0; i < servers.size(); i++) {
			result = reloadServer(servers.get(i)) && result;
		}
		return result;
	}

	public Boolean reloadServer(String serverName) {
		log.trace("reloadServer");
		Boolean result = true;
		List<Integer> ids = listRunningInstanceIds(serverName);

		for (int i = 0; i < ids.size(); i++) {
			result = reloadServerById(serverName, ids.get(i)) && result;
		}
		return result;
	}

	public Boolean reloadServerById(String serverName, int id) {
		log.trace("reloadServerById");
		return false;

	}

	public Element listServiceStatusById(String serverName, int id,
			String serviceName) {
		log.trace("listServiceStatusById");
		String command = "status";
		Response buf = null;
		String status = null;

		try {
			if (serviceName != null) {
				command = command + "," + serviceName + ",";
			}

			buf = callAdminService(serverName, id, command);
			if (buf != null) {
				byte[] received = ((X_OCTET) buf.getBuffer()).getByteArray();
				if (received[0] == '1') {
					status = new String(received, 1, received.length - 1);
					log.info("status is " + status);
					return stringToElement(status);
				}
			}
		} catch (ConnectionException e) {
			log.error("call server " + serverName + " id " + id
					+ " failed with " + e.getTperrno());
		} catch (Exception e) {
			log.error("response " + status + " error with " + e);
		}
		return null;
	}

	public MBeanServerConnection getBeanServerConnection() {
		log.trace("getBeanServerConnection");
		return beanServerConnection;
	}

	public void close() throws ConnectionException, IOException {
		log.info("Closed Administration Proxy");
		connection.close();
		c.close();
	}
}
