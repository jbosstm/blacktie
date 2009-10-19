package org.jboss.blacktie.administration;

import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;
import java.util.Properties;

import javax.ejb.ActivationConfigProperty;
import javax.ejb.MessageDriven;
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
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.conf.XMLEnvHandler;
import org.jboss.blacktie.jatmibroker.core.conf.XMLParser;
import org.jboss.blacktie.jatmibroker.xatmi.mdb.MDBBlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;
import org.jboss.ejb3.annotation.Depends;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.xml.sax.InputSource;

@MessageDriven(activationConfig = {
		@ActivationConfigProperty(propertyName = "destinationType", propertyValue = "javax.jms.Queue"),
		@ActivationConfigProperty(propertyName = "destination", propertyValue = "queue/BTStompAdmin") })
@Depends("jboss.messaging.destination:service=Queue,name=BTStompAdmin")
@javax.ejb.TransactionAttribute(javax.ejb.TransactionAttributeType.NOT_SUPPORTED)
public class BlacktieStompAdministrationService extends MDBBlacktieService
		implements javax.jms.MessageListener {
	private static final Logger log = LogManager
			.getLogger(BlacktieStompAdministrationService.class);

	private MBeanServerConnection beanServerConnection;


	public BlacktieStompAdministrationService() throws IOException,
			ConfigurationException, ConnectionException {
		super("BTStompAdmin");

		Properties prop = new Properties();
		XMLEnvHandler handler = new XMLEnvHandler("", prop);
		XMLParser xmlenv = new XMLParser(handler, "Environment.xsd");
		xmlenv.parse("Environment.xml");
		JMXServiceURL u = new JMXServiceURL(
				(String)prop.get("JMXURL"));
		JMXConnector c = JMXConnectorFactory.connect(u);
		beanServerConnection = c.getMBeanServerConnection();
	}

	boolean isDeployQueue(ObjectName objName, String serviceName)
			throws Exception {
		HashSet dests = (HashSet) beanServerConnection.getAttribute(objName,
				"Destinations");

		Iterator<Destination> it = dests.iterator();
		while (it.hasNext()) {
			Destination dest = it.next();
			if (dest instanceof Queue) {
				String qname = ((Queue) dest).getQueueName();
				log.debug("destination is " + qname);
				if (qname.equals(serviceName)) {
					log.debug("find serviceName " + serviceName);
					return true;
				}
			}
		}
		return false;
	}

	int consumerCount(String serviceName) throws Exception {
		ObjectName objName = new ObjectName(
				"jboss.messaging.destination:service=Queue,name=" + serviceName);
		Integer count = (Integer) beanServerConnection.getAttribute(objName,
				"ConsumerCount");
		Element security = (Element) beanServerConnection.getAttribute(objName,
				"SecurityConfig");
		log.debug(serviceName + " security config is " + printNode(security));
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

	void setSecurityConfig(ObjectName objName) {
		String security = "<security>\n"
				+ " <role name=\"guest\" read=\"true\" write=\"true\"/>\n"
				+ " <role name=\"publisher\" read=\"true\" write=\"true\" create=\"false\"/>\n"
				+ " <role name=\"durpublisher\" read=\"true\" write=\"true\" create=\"true\"/>\n"
				+ "</security>";

		try {
			Element element = stringToElement(security);
			Attribute attr = new Attribute("SecurityConfig", element);
			beanServerConnection.setAttribute(objName, attr);
		} catch (Throwable t) {
			log.error("Could not set security config " + t);
		}
	}

	int deployQueue(String serviceName) {
		int result = 0;

		try {
			ObjectName objName = new ObjectName(
					"jboss.messaging:service=ServerPeer");
			boolean queue = false;

			queue = isDeployQueue(objName, serviceName);
			if (queue == false) {
				beanServerConnection.invoke(objName, "deployQueue",
						new Object[] { serviceName, null }, new String[] {
								"java.lang.String", "java.lang.String" });
				ObjectName queueName = new ObjectName(
						"jboss.messaging.destination:service=Queue,name="
								+ serviceName);
				setSecurityConfig(queueName);
			}

			if(queue == false || !serviceName.contains("ADMIN") ) {
				result = 1;	
			} else if(consumerCount(serviceName) > 0){
				log.info("can not advertise ADMIN with same id");
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
			ObjectName objName = new ObjectName(
					"jboss.messaging:service=ServerPeer");
			if (isDeployQueue(objName, serviceName)) {
				beanServerConnection.invoke(objName, "undeployQueue",
						new Object[] { serviceName },
						new String[] { "java.lang.String" });
			}
			result = 1;
		} catch (Throwable t) {
			log.error("Could not undeploy queue of " + serviceName, t);
		}

		return result;
	}

	int decrementConsumer(String serviceName) {
		int consumerCounts;
		int result = 0;

		try {
			consumerCounts = consumerCount(serviceName);
			log.debug("Service " + serviceName + " has " + consumerCounts
					+ " consumers");
			if (consumerCounts == 1) {
				result = undeployQueue(serviceName);
			} else {
				// THERE ARE OTHER SERVERS STILL ALIVE
				result = 1;
			}
		} catch (Throwable t) {
			log.error("Could not get consumer counts of " + serviceName, t);
		}
		return result;
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		log.debug("Message received");
		Buffer recv = svcinfo.getBuffer();
		String string = new String(recv.getData());
		StringTokenizer st = new StringTokenizer(string, ",", false);
		String operation = st.nextToken();
		String serviceName = st.nextToken();
		byte[] success = new byte[1];
		String server = null;
		int k = -1;

		try {
			Properties prop = new Properties();
			XMLEnvHandler handler = new XMLEnvHandler("", prop);
			XMLParser xmlenv = new XMLParser(handler, "Environment.xsd");
			xmlenv.parse("Environment.xml");
			
			if ((k = serviceName.indexOf("ADMIN")) > 0) {
				String svcadm = serviceName.substring(0, k) + "ADMIN";
				server = (String) prop.get("blacktie." + svcadm + ".server");
			} else {
				server = (String) prop.get("blacktie." + serviceName + ".server");
			}

			if (server != null) {
				log.trace("Service " + serviceName + " exists for server: "
						+ server);
				if (operation.equals("tpunadvertise")) {
					log.trace("Unadvertising: " + serviceName);
					success[0] = (byte) undeployQueue(serviceName);
				} else if (operation.equals("tpadvertise")) {
					log.trace("Advertising: " + serviceName);
					success[0] = (byte) deployQueue(serviceName);
				} else if (operation.equals("decrementconsumer")) {
					log.trace("Decrement consumer: " + serviceName);
					success[0] = (byte) decrementConsumer(serviceName);
				} else {
					log.error("Unknow operation " + operation);
					success[0] = 0;
				}
			} else {
				log.error("Service " + serviceName
						+ " cannot be located for server");
				success[0] = 0;
			}

			Buffer buffer = new X_OCTET();
			buffer.setData(success);
			log.debug("Responding");
			return new Response(Connection.TPSUCCESS, 0, buffer, 1, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		} catch (ConfigurationException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
