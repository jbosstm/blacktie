package org.jboss.blacktie.jatmibroker.core.transport.hybrid;

import java.util.Properties;

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.AtmiBrokerEnvXML;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.Message;
import org.jboss.blacktie.jatmibroker.core.transport.Receiver;
import org.jboss.blacktie.jatmibroker.core.transport.Sender;
import org.jboss.blacktie.jatmibroker.core.transport.Transport;
import org.jboss.blacktie.jatmibroker.core.transport.TransportFactory;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public class TestConnection extends TestCase {
	private static final Logger log = LogManager
			.getLogger(TestConnection.class);
	private TransportFactory transportFactory;
	private Transport serviceTransport;
	private Transport clientTransport;

	public void setUp() throws ConnectionException, ConfigurationException {
		AtmiBrokerEnvXML xml = new AtmiBrokerEnvXML();
		Properties properties = xml.getProperties();

		transportFactory = TransportFactory.getTransportFactory(
				"JAVA_Converse", properties);
		serviceTransport = transportFactory.createTransport();
		clientTransport = transportFactory.createTransport();
	}

	public void tearDown() throws ConnectionException {
		clientTransport.close();
		serviceTransport.close();
		transportFactory.close();
	}

	public void test() throws ConnectionException {
		Receiver serviceDispatcher = serviceTransport
				.getReceiver("JAVA_Converse");
		Sender clientSender = clientTransport.getSender("JAVA_Converse");
		Receiver clientReceiver = clientTransport.createReceiver(1, null);
		clientSender.send(clientReceiver.getReplyTo(), (short) 1, 1, "hi"
				.getBytes(), 2, 0, 0, 0, "X_OCTET", null);
		Message receive = serviceDispatcher.receive(0);
		assertTrue(receive.len == 2);

		Sender serviceSender = serviceTransport.createSender(receive.replyTo);
		Receiver serviceReceiver = serviceTransport.createReceiver(1, null);

		log.info("Chatting");
		for (int i = 0; i < 100; i++) {
			serviceSender.send(serviceReceiver.getReplyTo(), (short) 1, 1,
					"chat".getBytes(), 4, 0, 0, 0, "X_OCTET", null);
			Message receive2 = clientReceiver.receive(0);
			assertTrue(receive2.len == 4);
		}
		log.info("Chatted");
	}
}
