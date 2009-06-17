package org.jboss.blacktie.administration;

import java.io.IOException;
import java.util.StringTokenizer;
import java.util.Properties;

import javax.ejb.ActivationConfigProperty;
import javax.ejb.MessageDriven;
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.conf.XMLEnvHandler;
import org.jboss.blacktie.jatmibroker.conf.XMLParser;
import org.jboss.blacktie.jatmibroker.mdb.MDBBlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.ejb3.annotation.Depends;

@MessageDriven(activationConfig = {
		@ActivationConfigProperty(propertyName = "destinationType", propertyValue = "javax.jms.Queue"),
		@ActivationConfigProperty(propertyName = "destination", propertyValue = "queue/BTStompAdmin") })
@Depends("jboss.messaging.destination:service=Queue,name=BTStompAdmin")
public class BlacktieStompAdministrationService extends MDBBlacktieService
		implements javax.jms.MessageListener {
	private static final Logger log = LogManager
			.getLogger(BlacktieStompAdministrationService.class);

	private MBeanServerConnection beanServerConnection;

	private Properties prop = new Properties();


	public BlacktieStompAdministrationService() throws IOException, ConfigurationException {
		super("BTStompAdmin");
		JMXServiceURL u = new JMXServiceURL(
				"service:jmx:rmi:///jndi/rmi://localhost:1090/jmxconnector");
		JMXConnector c = JMXConnectorFactory.connect(u);
		beanServerConnection = c.getMBeanServerConnection();
		XMLEnvHandler handler = new XMLEnvHandler("", prop);
		XMLParser xmlenv = new XMLParser(handler, "Environment.xsd");
		xmlenv.parse("Environment.xml");
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		Buffer recv = svcinfo.getBuffer();
		String string = new String(recv.getData());
		StringTokenizer st = new StringTokenizer(string, ",", false);
		String operation = st.nextToken();
		String serviceName = st.nextToken();
		byte[] success = new byte[1];
		String server = (String)prop.get("blacktie."+serviceName+".server");
		if (server != null) {
			log.debug("Service " + serviceName + " exists for server: " + server);
			if (operation.equals("tpunadvertise")) {
				log.debug("Unadvertising: " + serviceName);
				try {
					ObjectName name = new ObjectName(
							"jboss.messaging.destination:service=Queue,name="
									+ serviceName);
					beanServerConnection.invoke(name, "stop", null, null);
					success[0] = 1;
					log.info("Unadvertised: " + serviceName);
				} catch (Throwable t) {
					log.error("Could not advertise the service", t);
				}
			} else if (operation.equals("tpadvertise")) {
				log.debug("Advertising: " + serviceName);
				try {
					ObjectName name = new ObjectName(
							"jboss.messaging.destination:service=Queue,name="
									+ serviceName);
					beanServerConnection.invoke(name, "start", null, null);
					success[0] = 1;
					log.info("Advertised: " + serviceName);
				} catch (Throwable t) {
					log.error("Could not advertise the service", t);
				}
			}
		} else {
			log.error("Service " + serviceName + " cannot be located for server");
		}
		Buffer buffer = new Buffer(null, null);
		buffer.setData(success);
		return new Response((short) 0, 0, buffer, 1, 0);
	}
}
