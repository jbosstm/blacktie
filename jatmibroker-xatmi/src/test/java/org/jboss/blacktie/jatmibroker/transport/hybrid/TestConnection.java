package org.jboss.blacktie.jatmibroker.transport.hybrid;

import java.util.Properties;

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.AtmiBrokerClientXML;
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
	private TransportFactory serviceTransportFactory;
	private Transport serviceTransport;
	private TransportFactory clientTransportFactory;
	private Transport clientTransport;

	public void setUp() throws ConnectionException, ConfigurationException {
		AtmiBrokerClientXML xml = new AtmiBrokerClientXML();
		Properties properties = xml.getProperties();

		serviceTransportFactory = TransportFactory.loadTransportFactory(
				"TestOne", properties);
		serviceTransport = serviceTransportFactory.createTransport();

		clientTransportFactory = TransportFactory.loadTransportFactory(
				"TestOne", properties);
		clientTransport = clientTransportFactory.createTransport();
	}

	public void tearDown() throws ConnectionException {
		clientTransport.close();
		serviceTransport.close();
	}

	public void test() throws ConnectionException {
		Receiver serviceDispatcher = serviceTransport.getReceiver("TestOne");
		Sender clientSender = clientTransport.getSender("TestOne");
		Receiver clientReceiver = clientTransport.createReceiver();
		clientSender.send(clientReceiver.getReplyTo(), (short) 1, 1, "hi"
				.getBytes(), 2, 0, 0);
		Message receive = serviceDispatcher.receive(0);
		assertTrue(receive.len == 2);

		Sender serviceSender = serviceTransport.createSender(receive.replyTo);
		Receiver serviceReceiver = serviceTransport.createReceiver();

		log.info("Chatting");
		for (int i = 0; i < 100; i++) {
			serviceSender.send(serviceReceiver.getReplyTo(), (short) 1, 1,
					"chat".getBytes(), 4, 0, 0);
			Message receive2 = clientReceiver.receive(0);
			assertTrue(receive2.len == 4);
		}
		log.info("Chatted");
	}
}
