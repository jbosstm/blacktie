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

/**
 * This is the core proxy to forward requests to the individual servers.
 */
public class AdministrationProxy {
	private static final Logger log = LogManager
			.getLogger(AdministrationProxy.class);
	private Properties prop = new Properties();
	private JMXConnector c;
	private MBeanServerConnection beanServerConnection;
	private Connection connection;
	private List<String> servers;

	public static Boolean isDomainPause = false;

	public AdministrationProxy() throws IOException, ConfigurationException {
		log.debug("Administration Proxy");
		XMLEnvHandler handler = new XMLEnvHandler(prop);
		XMLParser xmlenv = new XMLParser(handler, "btconfig.xsd");
		xmlenv.parse("btconfig.xml");
		servers = (List<String>) prop.get("blacktie.domain.servers");
		ConnectionFactory cf = ConnectionFactory.getConnectionFactory();
		connection = cf.getConnection();
		JMXServiceURL u = new JMXServiceURL((String) prop.get("JMXURL"));
		c = JMXConnectorFactory.connect(u);
		beanServerConnection = c.getMBeanServerConnection();
		log.debug("Created Administration Proxy");
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

		String service = "." + serverName + id;

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

	public Boolean getDomainStatus() {
		return isDomainPause;
	}

	public Boolean pauseDomain() {
		log.trace("pauseDomain");
		Boolean result = true;
		List<String> servers = listRunningServers();

		for (int i = 0; i < servers.size(); i++) {
			result = pauseServer(servers.get(i)) && result;
		}

		if (result == true && isDomainPause == false) {
			isDomainPause = true;
			log.info("Domain pause");
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

		if (result == true && isDomainPause == true) {
			isDomainPause = false;
			log.info("Domain resume");
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
					if (qname.startsWith(".")) {
						String sname = qname.substring(1);
						sname = sname.replaceAll("[0-9]", "");
						if (servers.contains(sname)
								&& !runningServerList.contains(sname)) {
							runningServerList.add(sname);
						}
					}
				}
			}
		} catch (Exception e) {
			log.error("Caught an exception: " + e.getMessage(), e);
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
					if (qname.startsWith(".")) {
						String server = qname.substring(1);
						server = server.replaceAll("[0-9]", "");
						if (server.equals(serverName)) {
							qname = qname.substring(1);
							qname = qname.replaceAll("[A-Za-z]", "");
							ids.add(new Integer(qname));
						}
					}
				}
			}
		} catch (Exception e) {
			log.error("Caught an exception: " + e.getMessage(), e);
		}

		return ids;
	}

	public Element getServersStatus() {
		log.trace("getServersStatus");
		try {
			String status = "<servers>\n";

			for (String server : servers) {
				status += "\t<server>\n";
				status += "\t\t<name>" + server + "</name>\n";
				List<Integer> ids = listRunningInstanceIds(server);
				if (ids.size() > 0) {
					status += "\t\t<instances>\n";
					for (int i = 0; i < ids.size(); i++) {
						status += "\t\t\t<instance>\n";
						status += "\t\t\t\t<id>" + ids.get(i) + "</id>\n";
						status += "\t\t\t\t<status>1</status>\n";
						status += "\t\t\t</instance>\n";
					}
					status += "\t\t</instances>\n";
				}
				status += "\t</server>\n";
			}

			status += "</servers>";
			return stringToElement(status);
		} catch (Exception e) {
			log.error("Caught an exception: " + e.getMessage(), e);
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
			log.error("Caught an exception: " + e.getMessage(), e);
		}

		return status;
	}

	public Boolean advertise(String serverName, String serviceName) {
		log.trace("advertise");
		List<Integer> ids = listRunningInstanceIds(serverName);
		Boolean result = true;

		if (ids.size() == 0) {
			log.warn("Server was not running: " + serverName);
			return false;
		}

		for (int i = 0; i < ids.size(); i++) {
			result = advertise(serverName, ids.get(i), serviceName) && result;
			log.warn("Failed to advertise service at: " + ids.get(i));
		}

		return result;
	}

	public Boolean unadvertise(String serverName, String serviceName) {
		log.trace("unadvertise");
		List<Integer> ids = listRunningInstanceIds(serverName);
		Boolean result = true;

		if (ids.size() == 0) {
			log.warn("Server was not running: " + serverName);
			return false;
		}

		for (int i = 0; i < ids.size(); i++) {
			result = unadvertise(serverName, ids.get(i), serviceName) && result;
			log.warn("Failed to unadvertise service at: " + ids.get(i));
		}

		return result;
	}

	public Boolean shutdown(String serverName, int id) {
		log.trace("shutdown");
		if (servers.contains(serverName)) {
			String command = "serverdone";
			boolean shutdown = false;
			try {
				if (id == 0) {
					List<Integer> ids = listRunningInstanceIds(serverName);
					for (int i = 0; i < ids.size(); i++) {
						callAdminService(serverName, ids.get(i), command);
					}
				} else {
					callAdminService(serverName, id, command);
				}
				int timeout = 40;
				while (true) {
					List<Integer> ids = listRunningInstanceIds(serverName);
					if (id == 0 && ids.size() > 0 || ids.contains(id)) {
						try {
							Thread.sleep(3000);
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
						timeout--;
					} else {
						shutdown = true;
						break;
					}
					if (timeout == 0) {
						log.warn("Server did not shutdown in time: "
								+ serverName + ": " + id);
						break;
					}
				}
				return shutdown;
			} catch (ConnectionException e) {
				log.error("call server " + serverName + " id " + id
						+ " failed with " + e.getTperrno(), e);
				return false;
			} catch (RuntimeException e) {
				log.error("Could not shutdown server: " + e.getMessage(), e);
				throw e;
			}
		} else {
			log.error("Server not configured: " + serverName);
			return false;
		}
	}

	public String getResponseTimeById(String serverName, int id,
			String serviceName) {
		log.trace("getResponseTimeById");
		String command = "responsetime," + serviceName + ",";
		log.trace("response command is " + command);

		try {
			Response buf = callAdminService(serverName, id, command);
			if (buf != null) {
				byte[] received = ((X_OCTET) buf.getBuffer()).getByteArray();
				String result = new String(received, 1, received.length - 1);
				log.trace("response result is " + result);
				return result;
			}
		} catch (ConnectionException e) {
			log.error("call server " + serverName + " id " + id
					+ " failed with " + e.getTperrno(), e);
		} catch (RuntimeException e) {
			log.error("Could not get response time from server: "
					+ e.getMessage(), e);
			throw e;
		}
		return null;
	}

	public String getResponseTime(String serverName, String serviceName) {
		log.trace("getResponseTime");

		List<Integer> ids = listRunningInstanceIds(serverName);
		String responseTime;
		long min = 0;
		long avg = 0;
		long max = 0;
		long total = 0;

		for (int i = 0; i < ids.size(); i++) {
			responseTime = getResponseTimeById(serverName, ids.get(i),
					serviceName);
			String[] times = responseTime.split(",");

			if (times.length == 3) {
				long t = Long.valueOf(times[0]);
				if (min == 0 || t < min) {
					min = t;
				}

				t = Long.valueOf(times[2]);
				if (t > max) {
					max = t;
				}

				long counter = getServiceCounterById(serverName, ids.get(i),
						serviceName);
				t = Long.valueOf(times[1]);
				if (total != 0 || counter != 0) {
					avg = (avg * total + t * counter) / (total + counter);
				}
			}
		}

		return String.format("%d,%d,%d", min, avg, max);
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

	public long getErrorCounterById(String serverName, int id,
			String serviceName) {
		log.trace("getErrorCounterById");
		long counter = 0;
		String command = "error_counter," + serviceName + ",";

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

	public long getErrorCounter(String serverName, String serviceName) {
		log.trace("getErrorCounter");
		long counter = 0;
		List<Integer> ids = listRunningInstanceIds(serverName);

		for (int i = 0; i < ids.size(); i++) {
			counter += getErrorCounterById(serverName, ids.get(i), serviceName);
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
		log.debug("Closed Administration Proxy");
		connection.close();
		c.close();
	}

	public int getQueueDepth(String serverName, String serviceName) {
		Integer depth;
		try {
			ObjectName objName = new ObjectName(
					"jboss.messaging.destination:service=Queue,name="
							+ serviceName);
			depth = (Integer) beanServerConnection.getAttribute(objName,
					"MessageCount");
		} catch (Exception e) {
			log.error("getQueueDepth failed with " + e);
			return -1;
		}
		return depth.intValue();
	}

	public String getServerName(String serviceName) {
		return prop.getProperty("blacktie." + serviceName + ".server");
	}

	public String getServerVersionById(String serverName, int id) {
		log.trace("getServerVersionById");
		String command = "version";
		Response buf = null;
		String version = null;

		try {
			buf = callAdminService(serverName, id, command);
			if (buf != null) {
				byte[] received = ((X_OCTET) buf.getBuffer()).getByteArray();
				if (received[0] == '1') {
					version = new String(received, 1, received.length - 1);
					log.debug("version is " + version);
				}
			}
		} catch (ConnectionException e) {
			log.error("call server " + serverName + " id " + id
					+ " failed with " + e.getTperrno(), e);
		}
		return version;
	}
}
