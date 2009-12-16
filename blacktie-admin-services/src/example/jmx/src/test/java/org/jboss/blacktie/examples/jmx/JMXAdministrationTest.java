package org.jboss.blacktie.examples.jmx;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Properties;

import javax.management.MBeanServerConnection;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

import junit.framework.TestCase;

import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.conf.XMLEnvHandler;
import org.jboss.blacktie.jatmibroker.core.conf.XMLParser;

/**
 * As this is an interactive test, the forkMode must be set to none is vital for
 * the input to be received by maven
 */
public class JMXAdministrationTest extends TestCase {

	public void test() throws IOException, ConfigurationException {
		InputStreamReader isr = new InputStreamReader(System.in);
		BufferedReader br = new BufferedReader(isr);
		System.out.println("usage: mvn test");
		System.out
				.println("warning: forkMode must be set to none, please see README");

		prompt(br, "Please start JBoss Application Server");
		prompt(br, "Please ensure no XATMI servers are running");

		Properties prop = new Properties();
		XMLEnvHandler handler = new XMLEnvHandler("", prop);
		XMLParser xmlenv = new XMLParser(handler, "Environment.xsd");
		xmlenv.parse("Environment.xml", true);
		JMXServiceURL u = new JMXServiceURL((String) prop.get("JMXURL"));
		JMXConnector c = JMXConnectorFactory.connect(u);
		MBeanServerConnection beanServerConnection = c
				.getMBeanServerConnection();

		prompt(br, "Press return when you have started a second server");

	}

	private void prompt(BufferedReader br, String prompt) throws IOException {
		System.out.println(prompt + "...");
		String readLine = br.readLine();
	}
}
