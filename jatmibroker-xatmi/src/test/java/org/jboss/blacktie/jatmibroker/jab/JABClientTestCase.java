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
package org.jboss.blacktie.jatmibroker.jab;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;

import junit.framework.TestCase;

import org.jboss.blacktie.jatmibroker.RunServer;

public class JABClientTestCase extends TestCase {
	private RunServer runServer = new RunServer();

	public void setUp() throws InterruptedException {
		runServer.serverinit();
	}

	public void tearDown() {
		runServer.serverdone();
	}

	public void test_tpcall_x_octet() throws Exception {
		JABSessionAttributes aJabSessionAttributes = new JABSessionAttributes(
				null);
		JABSession aJabSession = new JABSession(aJabSessionAttributes);
		JABTransaction transaction = new JABTransaction(aJabSession, 5000);
		JABRemoteService aJabService = new JABRemoteService(aJabSession,
				"tpcall_x_octet");
		byte[] toSend = "HOWS IT GOING DUDE????!!!!".getBytes();
		aJabService.setBuffer("X_OCTET", toSend, toSend.length);
		aJabService.call(transaction);
		transaction.commit();
		aJabSession.endSession();
		String expectedString = "BAR SAYS HELLO";
		String responseString = new String(aJabService.getResponseData());
		assertEquals(expectedString, responseString);
	}

	public void test_tpcall_x_c_type() throws Exception {
		JABSessionAttributes aJabSessionAttributes = new JABSessionAttributes(
				null);
		JABSession aJabSession = new JABSession(aJabSessionAttributes);
		JABTransaction transaction = new JABTransaction(aJabSession, 5000);
		JABRemoteService aJabService = new JABRemoteService(aJabSession,
				"tpcall_x_c_type");

		// Assemble the message
		ByteArrayOutputStream baos = new ByteArrayOutputStream(512);
		DataOutputStream dos = new DataOutputStream(baos);
		dos.writeInt(222);
		dos.writeShort((short) 33);
		dos.writeLong(11l);
		dos.writeChar('c');
		dos.writeFloat(444.97f);
		dos.writeDouble(7.7d);
		dos.writeUTF("tpcall_x_c_type");
		byte[] data = baos.toByteArray();

		aJabService.setBuffer("X_C_TYPE", data, data.length);
		aJabService.call(transaction);
		transaction.commit();
		aJabSession.endSession();

		byte[] response = aJabService.getResponseData();
		ByteArrayInputStream bais = new ByteArrayInputStream(response);
		DataInputStream dis = new DataInputStream(bais);
		assertEquals(222, dis.readInt());
		assertEquals(33, dis.readShort());
		assertEquals(11, dis.readLong());
		assertEquals('c', dis.readChar());
		assertEquals(444.97, dis.readFloat());
		assertEquals(7.7, dis.readDouble());
		assertEquals("tpcall_x_c_type", dis.readUTF());
	}
}
