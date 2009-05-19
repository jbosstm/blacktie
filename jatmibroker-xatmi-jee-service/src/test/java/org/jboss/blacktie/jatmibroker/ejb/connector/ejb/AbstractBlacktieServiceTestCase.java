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
package org.jboss.blacktie.jatmibroker.ejb.connector.ejb;

import java.util.Properties;

import junit.framework.TestCase;

import org.jboss.blacktie.jatmibroker.core.CoreException;
import org.jboss.blacktie.jatmibroker.server.AtmiBrokerServer;
import org.jboss.blacktie.jatmibroker.xatmi.Connector;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectorException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectorFactory;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.buffers.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.buffers.X_OCTET;
import org.jboss.blacktie.jatmibroker.xatmi.impl.ConnectorFactoryImpl;

public class AbstractBlacktieServiceTestCase extends TestCase {
	private Connector connector;
	private AtmiBrokerServer server;

	public AbstractBlacktieServiceTestCase() throws ConnectorException {
		System.setProperty("blacktie.server.name", "ejb-connector-tests");
	}

	public void setUp() throws ConnectorException, CoreException {
		this.server = new AtmiBrokerServer();
		this.server.tpadvertise("EchoService", EchoServiceTestService.class);
		ConnectorFactory connectorFactory = ConnectorFactoryImpl
				.getConnectorFactory();
		connector = connectorFactory.getConnector();
	}

	public void tearDown() throws ConnectorException {
		connector.close();
		server.tpunadvertise("EchoService");
	}

	public void testWithProperties() throws ConnectorException {
		Properties properties = new Properties();
		properties.put("blacktie.orb.args", "2");
		properties.put("blacktie.orb.arg.1", "-ORBInitRef");
		properties.put("blacktie.orb.arg.2",
				"NameService=corbaloc::localhost:3528/NameService");
		properties.put("blacktie.domain.name", "jboss");
		properties.put("blacktie.server.name", "ejb-connector-tests");
		String serviceName = "EchoService";
		ConnectorFactory connectorFactory = ConnectorFactoryImpl
				.getConnectorFactory(properties);
		Connector connector = connectorFactory.getConnector();
		byte[] echo = "echo".getBytes();
		Buffer buffer = new X_OCTET(echo.length);
		buffer.setData(echo);
		Response response = connector.tpcall(serviceName, buffer, 0);
		Buffer responseBuffer = response.getResponse();
		byte[] responseData = responseBuffer.getData();
		assertEquals("echo", new String(responseData));
	}

	public void testWithDefaultProperties() throws ConnectorException {
		String serviceName = "EchoService";
		ConnectorFactory connectorFactory = ConnectorFactoryImpl
				.getConnectorFactory();
		Connector connector = connectorFactory.getConnector();
		byte[] echo = "echo".getBytes();
		Buffer buffer = new X_OCTET(echo.length);
		buffer.setData(echo);
		Response response = connector.tpcall(serviceName, buffer, 0);
		Buffer responseBuffer = response.getResponse();
		byte[] responseData = responseBuffer.getData();
		assertEquals("echo", new String(responseData));
	}

}
