package org.jboss.blacktie.administration;

import java.io.IOException;
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
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.conf.XMLEnvHandler;
import org.jboss.blacktie.jatmibroker.core.conf.XMLParser;
import org.jboss.blacktie.jatmibroker.xatmi.mdb.MDBBlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.ejb3.annotation.Depends;

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

	private Properties prop = new Properties();

	public BlacktieStompAdministrationService() throws IOException,
			ConfigurationException, ConnectionException {
		super("BTStompAdmin");
		JMXServiceURL u = new JMXServiceURL(
				"service:jmx:rmi:///jndi/rmi://localhost:1090/jmxconnector");
		JMXConnector c = JMXConnectorFactory.connect(u);
		beanServerConnection = c.getMBeanServerConnection();

		XMLEnvHandler handler = new XMLEnvHandler("", prop);
		XMLParser xmlenv = new XMLParser(handler, "Environment.xsd");
		xmlenv.parse("Environment.xml");
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
		return count.intValue();
	}

	int deployQueue(String serviceName) {
		int result = 0;

		try {
			ObjectName objName = new ObjectName(
					"jboss.messaging:service=ServerPeer");
			if (isDeployQueue(objName, serviceName) == false) {
				beanServerConnection.invoke(objName, "deployQueue",
						new Object[] { serviceName, null },
						new String[] { "java.lang.String",
							"java.lang.String" });
			}
			result = 1;
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
			log.debug("Service " + serviceName + " has " + consumerCounts + " consumers");
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

		if ((k = serviceName.indexOf("ADMIN")) > 0) {
			String svcadm = serviceName.substring(0, k) + "ADMIN";
			server = (String) prop.get("blacktie." + svcadm + ".server");
		} else {
			server = (String) prop.get("blacktie." + serviceName + ".server");
		}

		if (server != null) {
			log.trace("Service " + serviceName + " exists for server: " + server);
			if (operation.equals("tpunadvertise")) {
				log.trace("Unadvertising: " + serviceName);
				success[0] = (byte)undeployQueue(serviceName);
			} else if (operation.equals("tpadvertise")) {
				log.trace("Advertising: " + serviceName);
				success[0] = (byte)deployQueue(serviceName);
			} else if (operation.equals("decrementconsumer")) {
				log.trace("Decrement consumer: " + serviceName);
				success[0] = (byte)decrementConsumer(serviceName);
			} else {
				log.error("Unknow operation " + operation);
				success[0] = 0;
			}
		} else {
			log.error("Service " + serviceName
					+ " cannot be located for server");
			success[0] = 0;
		}

		Buffer buffer = new Buffer(null, null);
		buffer.setData(success);
		log.debug("Responding");
		return new Response((short) 0, 0, buffer, 1, 0);
	}
}
