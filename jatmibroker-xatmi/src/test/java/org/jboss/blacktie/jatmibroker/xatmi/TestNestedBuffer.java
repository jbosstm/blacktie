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
package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;

import junit.framework.TestCase;

public class TestNestedBuffer extends TestCase {
	private static final Logger log = LogManager
			.getLogger(TestNestedBuffer.class);
	
	private Connection connection;
	
	public void setUp() throws ConnectionException, ConfigurationException {
		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();
	}
	
	public void tearDown() throws ConnectionException, ConfigurationException {
		connection.close();
	}
	
	public void test() throws ConnectionException {
		log.info("TestNestedBuffer::test");
		BT_NBF buffer = (BT_NBF) connection.tpalloc("BT_NBF", "employee", 0);
		assertFalse(buffer.btaddattribute("id", new Integer(1001)));
		assertTrue(buffer.btaddattribute("name", "zhfeng"));
		assertTrue(buffer.btaddattribute("id", new Long(1001)));
		
		log.info(new String(buffer.serialize()));
		
		Object obj = buffer.btgetattribute("id", 0);
		assertTrue("java.lang.Long".equals(obj.getClass().getName()));
		assertTrue(((Long)obj).longValue() == 1001);
		
		obj = buffer.btgetattribute("id", 1);
		assertTrue(obj == null);
		
		obj = buffer.btgetattribute("name", 0);
		assertTrue("java.lang.String".equals(obj.getClass().getName()));
		assertTrue("zhfeng".equals((String)(obj)));
		
		BT_NBF test = (BT_NBF) connection.tpalloc("BT_NBF", "test", 0);
		assertTrue(test.btaddattribute("employee", buffer));
		//log.info(new String(test.serialize()));
		
		obj = test.btgetattribute("employee", 0);
		assertTrue("org.jboss.blacktie.jatmibroker.xatmi.BT_NBF".equals(obj.getClass().getName()));
		BT_NBF employee = (BT_NBF)obj;
		String name = (String)employee.btgetattribute("name", 0);
		assertTrue("zhfeng".equals(name));
		Long id = (Long)employee.btgetattribute("id", 0);
		assertTrue(id.longValue() == 1001);
	}
	
	public void testDel() throws ConnectionException {
		log.info("TestNestedBuffer::testDel");
		BT_NBF buffer = (BT_NBF) connection.tpalloc("BT_NBF", "employee", 0);
		buffer.btaddattribute("id", new Long(1001));
		buffer.btaddattribute("id", new Long(1002));
		
		assertTrue(buffer.btdelattribute("id", 1));
		
		assertFalse(buffer.btdelattribute("unknow", 0));
	}
}
