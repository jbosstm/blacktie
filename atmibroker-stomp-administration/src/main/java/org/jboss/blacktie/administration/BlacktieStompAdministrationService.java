package org.jboss.blacktie.administration;

import java.io.IOException;
import java.util.StringTokenizer;

import javax.ejb.ActivationConfigProperty;
import javax.ejb.MessageDriven;
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.mdb.MDBBlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.ejb3.annotation.Depends;

@MessageDriven(activationConfig = {
		@ActivationConfigProperty(propertyName = "destinationType", propertyValue = "javax.jms.Queue"),
		@ActivationConfigProperty(propertyName = "destination", propertyValue = "queue/BlacktieStompAdministrationService") })
@Depends("jboss.mq.destination:service=Queue,name=BlacktieStompAdministrationService")
public class BlacktieStompAdministrationService extends MDBBlacktieService
		implements javax.jms.MessageListener {
	private static final Logger log = LogManager
			.getLogger(BlacktieStompAdministrationService.class);

	private MBeanServerConnection beanServerConnection;

	public BlacktieStompAdministrationService() throws IOException {
		super("BlacktieStompAdministrationService");
		JMXServiceURL u = new JMXServiceURL(
				"service:jmx:rmi:///jndi/rmi://localhost:8080/jmxrmi");
		JMXConnector c = JMXConnectorFactory.connect(u);
		beanServerConnection = c.getMBeanServerConnection();
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		Buffer recv = svcinfo.getBuffer();
		String string = new String(recv.getData());
		StringTokenizer st = new StringTokenizer(string, ",", false);
		String operation = st.nextToken();
		String serviceName = st.nextToken();
		byte[] success = new byte[1];
		if (operation.equals("tpunadvertise")) {
			try {
				ObjectName name = new ObjectName(
						"jboss.messaging.destination:service=Queue,name="
								+ serviceName);
				beanServerConnection.invoke(name, "stop", null, null);
				success[0] = 1;
			} catch (Throwable t) {
				log.error("Could not unadvertise the service");
			}
		} else if (operation.equals("tpadvertise")) {
			try {
				ObjectName name = new ObjectName(
						"jboss.messaging.destination:service=Queue,name="
								+ serviceName);
				beanServerConnection.invoke(name, "start", null, null);
				success[0] = 1;
			} catch (Throwable t) {
				log.error("Could not unadvertise the service");
			}
		}

		byte[] rcvd = svcinfo.getBuffer().getData();
		Buffer buffer = new Buffer(null, null);
		buffer.setData(success);
		return new Response((short) 0, 0, buffer, 1, 0);
	}
}
