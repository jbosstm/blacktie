/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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
/*
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT
package org.jboss.blacktie.jatmibroker.core;

import java.util.Properties;

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.proxy.AtmiBrokerServer;
import org.jboss.blacktie.jatmibroker.core.proxy.AtmiBrokerServiceManager;

public class ServiceManagerProxyTest extends TestCase {
	private static final Logger log = LogManager.getLogger(ServiceManagerProxyTest.class);

	public void setUp() throws InterruptedException {
		RunServer.startInstance();
	}

	public void tearDown() {
		RunServer.stopInstance();
	}

	public void test() throws Exception {
		Properties properties = new Properties();
		properties.put("blacktie.company.name", "default");
		properties.put("blacktie.server.name", "foo");
		properties.put("blacktie.orb.args", "2");
		properties.put("blacktie.orb.arg.1", "-ORBInitRef");
		properties.put("blacktie.orb.arg.2", "NameService=corbaloc::localhost:3528/NameService");

		AtmiBrokerServer proxy = AtmiBrokerServerImpl.getProxy(properties, "", "");
		AtmiBrokerServiceManager serviceManagerControl = proxy.getServiceManagerProxy("BAR");

		String aString = "Hello from Java Land";
		int flags = 0;
		AtmiBroker.octetSeqHolder odata = new AtmiBroker.octetSeqHolder();
		org.omg.CORBA.IntHolder olen = new org.omg.CORBA.IntHolder();

		serviceManagerControl.send_data(null, false, aString.getBytes(), aString.length(), flags, 0, null);
		log.debug("Bar ServiceManager service_request response is " + new String(odata.value));
		log.debug("Bar ServiceManager service_request size of response is " + olen.value);
		assertEquals(odata.value, "BAR");
		AtmiBrokerServerImpl.discardOrb();
	}
}
