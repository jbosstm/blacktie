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
package org.jboss.blacktie.example.mdb;

import org.jboss.ejb3.annotation.Depends;

import javax.ejb.ActivationConfigProperty;
import javax.ejb.MessageDriven;

import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.mdb.MDBBlacktieService;

@javax.ejb.TransactionAttribute(javax.ejb.TransactionAttributeType.NOT_SUPPORTED)
@MessageDriven(activationConfig = {
		@ActivationConfigProperty(propertyName = "destinationType", propertyValue = "javax.jms.Queue"),
		@ActivationConfigProperty(propertyName = "destination", propertyValue = "queue/EchoService") })
@Depends("jboss.messaging.destination:service=Queue,name=EchoService")
public class EchoServiceTestService extends MDBBlacktieService implements
		javax.jms.MessageListener {

	public EchoServiceTestService() {
		super("EchoService");
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		String rcvd = (String) svcinfo.getBuffer().getData();
		Buffer buffer = new Buffer(null, null);
		try {
			buffer.setData(rcvd);
		} catch (ConnectionException e2) {
			rcvd = "";
			e2.printStackTrace();
		}
		return new Response((short) 0, 0, buffer, rcvd.length(), 0);
	}
}
