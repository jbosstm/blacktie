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
import java.io.StringReader;
import java.io.StringWriter;
import java.util.Hashtable;
import java.util.List;
import java.util.Properties;
import java.util.StringTokenizer;

import javax.ejb.ActivationConfigProperty;
import javax.ejb.MessageDriven;
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.narayana.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.narayana.blacktie.jatmibroker.core.conf.XMLParser;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.Response;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.X_OCTET;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.mdb.MDBBlacktieService;
import org.jboss.ejb3.annotation.ResourceAdapter;
import org.jboss.narayana.blacktie.administration.core.AdministrationProxy;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.xml.sax.InputSource;

@MessageDriven(activationConfig = {
		@ActivationConfigProperty(propertyName = "destinationType", propertyValue = "javax.jms.Queue"),
		@ActivationConfigProperty(propertyName = "destination", propertyValue = "queue/BTR_BTStompAdmin") })
// @Depends("org.hornetq:module=JMS,name=\"BTR_BTStompAdmin\",type=Queue")
@javax.ejb.TransactionAttribute(javax.ejb.TransactionAttributeType.NOT_SUPPORTED)
@ResourceAdapter("hornetq-ra.rar")
public class BlacktieStompAdministrationService extends MDBBlacktieService
		implements javax.jms.MessageListener {
	private static final Logger log = LogManager
			.getLogger(BlacktieStompAdministrationService.class);

	private MBeanServerConnection beanServerConnection;
	private Properties prop = new Properties();

	public static Hashtable<String, Long> QUEUE_CREATION_TIMES = new Hashtable<String, Long>();

	public BlacktieStompAdministrationService() throws IOException,
			ConfigurationException {
		XMLParser.loadProperties("btconfig.xsd", "btconfig.xml", prop);
		beanServerConnection = org.jboss.mx.util.MBeanServerLocator
				.locateJBoss();
	}

	boolean isDeployQueue(String serviceName) throws Exception {
		ObjectName objName = new ObjectName(
				"org.hornetq:module=JMS,type=Server");
		String[] queues = (String[]) beanServerConnection.getAttribute(objName,
				"QueueNames");
		String[] topics = (String[]) beanServerConnection.getAttribute(objName,
				"TopicNames");

		log.trace(serviceName);
		boolean conversational = false;
		if (!serviceName.startsWith(".")) {
			conversational = (Boolean) prop.get("blacktie." + serviceName
					+ ".conversational");
		}
		String prefix = null;
		if (conversational) {
			prefix = "BTC_";
		} else {
			prefix = "BTR_";
		}
		for (int i = 0; i < queues.length; i++) {
			String qname = queues[i];
			log.trace("queue is " + qname);
			if (qname.equals(prefix + serviceName)) {
				log.debug("find serviceName " + serviceName + " in Queues");
				return true;
			}
		}
		
		for (int i = 0; i < topics.length; i++) {
			String tname = topics[i];
			log.trace("topic is " + tname);
			if (tname.equals(prefix + serviceName)) {
				log.debug("find serviceName " + serviceName + " in Topics");
				return true;
			}
		}
		
		log.trace("did not find serviceName " + serviceName);
		return false;
	}

	int consumerCount(String serviceName) throws Exception {
		log.trace("consCount" + serviceName);
		boolean conversational = false;
		String type = "queue";
		
		if (!serviceName.startsWith(".")) {
			conversational = (Boolean) prop.get("blacktie." + serviceName
					+ ".conversational");
			type = (String) prop.getProperty("blacktie." + serviceName
					+ ".type");
		}
		String prefix = null;
		if (conversational) {
			prefix = "BTC_";
		} else {
			prefix = "BTR_";
		}
		type = type.substring(0,1).toUpperCase() + type.substring(1);
		
		ObjectName objName = new ObjectName("org.hornetq:module=JMS,name=\""
				+ prefix + serviceName + "\",type=" + type);
		
		String attribute = "ConsumerCount";
		if(type.equals("Topic")) {
			attribute = "SubscriptionCount";
		}
		Integer count = (Integer) beanServerConnection.getAttribute(objName, attribute);
		log.debug("consCount" + serviceName + " " + count.intValue());
		return count.intValue();
	}

	Element stringToElement(String s) throws Exception {
		StringReader sreader = new StringReader(s);
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		DocumentBuilder parser = factory.newDocumentBuilder();
		Document doc = parser.parse(new InputSource(sreader));
		return doc.getDocumentElement();
	}

	String printNode(Node node) {
		try {
			// Set up the output transformer
			TransformerFactory transfac = TransformerFactory.newInstance();
			Transformer trans = transfac.newTransformer();
			trans.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
			trans.setOutputProperty(OutputKeys.INDENT, "yes");

			// Print the DOM node

			StringWriter sw = new StringWriter();
			StreamResult result = new StreamResult(sw);
			DOMSource source = new DOMSource(node);
			trans.transform(source, result);
			String xmlString = sw.toString();

			return xmlString;
		} catch (TransformerException e) {
			log.error(e);
		}
		return null;
	}

	int deployQueue(String serviceName, String version) {
		log.trace("deployQueue: " + serviceName + " version: " + version);

		if (version == null
				|| !version.equals(prop.getProperty("blacktie.domain.version"))) {
			log.warn("Blacktie Domain version "
					+ prop.getProperty("blacktie.domain.version")
					+ " not match server " + version);
			return 4;
		}

		int result = 0;
		// Long currentTime = QUEUE_CREATION_TIMES.get(serviceName);

		try {
			boolean queue = false;

			queue = isDeployQueue(serviceName);
			log.debug("Queue " + serviceName + " was created: " + queue);
			boolean created = queue;
			if (queue == false) {
				synchronized (QUEUE_CREATION_TIMES) {
					log.trace(serviceName);
					boolean conversational = false;
					String type = "queue";
					if (!serviceName.startsWith(".")) {
						conversational = (Boolean) prop.get("blacktie."
								+ serviceName + ".conversational");
						type = (String) prop.get("blacktie." 
								+ serviceName + ".type");
					}
					String prefix = null;
					if (conversational) {
						prefix = "BTC_";
					} else {
						prefix = "BTR_";
					}
					QUEUE_CREATION_TIMES.put(serviceName,
							System.currentTimeMillis());
					log.trace(serviceName);
					ObjectName objName = new ObjectName(
							"org.hornetq:module=JMS,type=Server");
					
					String op = "createQueue";
					if(type.equals("topic")) {
						op = "createTopic";
					}
					created = (Boolean) beanServerConnection.invoke(objName,
							op, new Object[] { prefix + serviceName,
									"/" + type + "/" + prefix + serviceName },
							new String[] { "java.lang.String",
									"java.lang.String" });
				}
			}
			// QUEUE_CREATION_TIMES.put(serviceName, currentTime);
			if (!queue || !serviceName.contains(".")) {
				result = 1;
				if (AdministrationProxy.isDomainPause
						&& serviceName.contains(".")) {
					log.info("Domain is pause");
					result = 3;
				}
			} else if (serviceName.contains(".") && created && consumerCount(serviceName) > 0) {
				log.warn("can not advertise ADMIN with same id: " + serviceName);
				result = 2;
			} else if (AdministrationProxy.isDomainPause) {
				log.info("Domain is pause");
				result = 3;
			} else {
				result = 1;
			}
		} catch (Throwable t) {
			log.error("Could not deploy queue of " + serviceName, t);
		}

		return result;
	}

	int undeployQueue(String serviceName) {
		int result = 0;

		try {
			if (isDeployQueue(serviceName)) {
				log.trace(serviceName);
				boolean conversational = false;
				String type = "queue";
				if (!serviceName.startsWith(".")) {
					conversational = (Boolean) prop.get("blacktie."
							+ serviceName + ".conversational");
					type = (String) prop.get("blacktie." + serviceName
							+ ".type");
				}
				String prefix = null;
				if (conversational) {
					prefix = "BTC_";
				} else {
					prefix = "BTR_";
				}
				
				String op = "destroyQueue";
				if(type.equals("topic")) {
					op = "destroyTopic";
				}
				
				ObjectName objName = new ObjectName(
						"org.hornetq:module=JMS,type=Server");
				beanServerConnection.invoke(objName, op,
						new Object[] { prefix + serviceName },
						new String[] { "java.lang.String" });
			}
			result = 1;
		} catch (Throwable t) {
			log.error("Could not undeploy queue of " + serviceName, t);
		}

		return result;
	}

	int decrementConsumer(String serviceName) {
		log.trace("decrement");
		int consumerCounts;
		int result = 0;

		try {
			consumerCounts = consumerCount(serviceName);
			if (consumerCounts < 1) {
				result = undeployQueue(serviceName);
				log.debug(serviceName + " undeployed");
			} else {
				// THERE ARE OTHER SERVERS STILL ALIVE
				result = 1;
				log.debug(serviceName + " still has " + consumerCounts
						+ " consumers");
			}
		} catch (Throwable t) {
			log.error("Could not get consumer counts of " + serviceName, t);
		}
		return result;
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		log.debug("Message received");
		X_OCTET recv = (X_OCTET) svcinfo.getBuffer();
		String string = new String(recv.getByteArray());
		StringTokenizer st = new StringTokenizer(string, ",", false);
		String operation = st.nextToken();
		String serverName = st.nextToken();
		String serviceName = st.nextToken();

		byte[] success = new byte[1];
		String server = null;

		try {
			if (serviceName.indexOf(".") > -1) {
				server = serviceName.substring(1);
				server = server.replaceAll("[0-9]", "");
				List<String> servers = (List<String>) prop
						.get("blacktie.domain.servers");
				if (servers.contains(server) == false) {
					log.warn("Could not find the server to advertise for: "
							+ server);
					server = null;
				} else {
					log.trace("Located server: " + server);
				}
			} else {
				server = (String) prop.get("blacktie." + serviceName
						+ ".server");
			}

			if (server != null && server.equals(serverName)) {
				log.trace("Service " + serviceName + " exists for server: "
						+ server);
				if (operation.equals("tpadvertise")) {
					log.trace("Advertising: " + serviceName);
					String version = st.nextToken();
					success[0] = (byte) deployQueue(serviceName, version);
				} else if (operation.equals("decrementconsumer")) {
					log.trace("Decrement consumer: " + serviceName);
					success[0] = (byte) decrementConsumer(serviceName);
				} else {
					log.error("Unknown operation " + operation);
					success[0] = 0;
				}
			} else {
				log.error("Service " + serviceName
						+ " cannot be located for server");
				success[0] = 0;
			}

			X_OCTET buffer = (X_OCTET) svcinfo.getConnection().tpalloc(
					"X_OCTET", null, 1);
			buffer.setByteArray(success);
			log.debug("Responding");
			return new Response(Connection.TPSUCCESS, 0, buffer, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0);
		}
	}
}
