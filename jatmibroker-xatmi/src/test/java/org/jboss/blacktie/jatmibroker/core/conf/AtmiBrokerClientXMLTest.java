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
package org.jboss.blacktie.jatmibroker.core.conf;

import java.util.Properties;

import junit.framework.TestCase;

public class AtmiBrokerClientXMLTest extends TestCase {

	public void test() throws Exception {
		AtmiBrokerEnvXML clientDesc = new AtmiBrokerEnvXML();
		Properties prop = clientDesc.getProperties();

		String domain = "fooapp";
		String transid = "TransactionManagerService.OTS";
		String args = "2";
		String arg1 = "-ORBInitRef";
		String arg2 = "NameService=corbaloc::";
		String arg3 = ":3528/NameService";

		assertTrue(domain.equals(prop.getProperty("blacktie.domain.name")));
		assertTrue(transid.equals(prop.getProperty("blacktie.trans.factoryid")));
		assertTrue(args.equals(prop.getProperty("blacktie.orb.args")));
		assertTrue(arg1.equals(prop.getProperty("blacktie.orb.arg.1")));
		assertTrue(((String) prop.getProperty("blacktie.orb.arg.2"))
				.startsWith(arg2));
		assertTrue(((String) prop.getProperty("blacktie.orb.arg.2"))
				.endsWith(arg3));
	}
}
