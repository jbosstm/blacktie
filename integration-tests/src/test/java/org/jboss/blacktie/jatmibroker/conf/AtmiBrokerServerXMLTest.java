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
package org.jboss.blacktie.jatmibroker.conf;

import java.util.Properties;

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class AtmiBrokerServerXMLTest extends TestCase {
	private static final Logger log = LogManager
			.getLogger(AtmiBrokerServerXMLTest.class);

	public void setUp() throws InterruptedException {
	}

	public void tearDown() {
	}

	public void test() throws Exception {
		AtmiBrokerServerXML serverDesc = new AtmiBrokerServerXML("foo");
		Properties prop = serverDesc.getProperties("win32");

		assertTrue("fooapp".equals(prop.getProperty("blacktie.domain.name")));
		assertTrue("TransactionManagerService.OTS".equals(prop
				.getProperty("blacktie.trans.factoryid")));
		assertTrue("2".equals(prop.getProperty("blacktie.orb.args")));
		assertTrue("-ORBInitRef".equals(prop.getProperty("blacktie.orb.arg.1")));
		assertTrue("NameService=corbaloc::localhost:3528/NameService"
				.equals(prop.getProperty("blacktie.orb.arg.2")));
	}

	public void testEnv() throws Exception {
		AtmiBrokerServerXML serverDesc = new AtmiBrokerServerXML("foo");
		Properties prop = serverDesc.getProperties();

		assertTrue("fooapp".equals(prop.getProperty("blacktie.domain.name")));
		assertTrue("TransactionManagerService.OTS".equals(prop
				.getProperty("blacktie.trans.factoryid")));
		assertTrue("2".equals(prop.getProperty("blacktie.orb.args")));
		assertTrue("-ORBInitRef".equals(prop.getProperty("blacktie.orb.arg.1")));
		assertTrue("NameService=corbaloc::localhost:3528/NameService"
				.equals(prop.getProperty("blacktie.orb.arg.2")));
	}
}
