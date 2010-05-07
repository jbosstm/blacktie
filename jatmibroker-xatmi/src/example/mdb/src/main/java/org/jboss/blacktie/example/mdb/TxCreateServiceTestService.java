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

import javax.ejb.ActivationConfigProperty;
import javax.ejb.MessageDriven;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.JtsTransactionImple;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionFactory;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;
import org.jboss.blacktie.jatmibroker.xatmi.mdb.MDBBlacktieService;
import org.jboss.ejb3.annotation.Depends;

@javax.ejb.TransactionAttribute(javax.ejb.TransactionAttributeType.NOT_SUPPORTED)
@MessageDriven(activationConfig = {
		@ActivationConfigProperty(propertyName = "destinationType", propertyValue = "javax.jms.Queue"),
		@ActivationConfigProperty(propertyName = "destination", propertyValue = "queue/TxCreateService") })
@Depends("jboss.messaging.destination:service=Queue,name=TxCreateService")
public class TxCreateServiceTestService extends MDBBlacktieService implements
		javax.jms.MessageListener {
	private static final Logger log = LogManager
			.getLogger(TxCreateServiceTestService.class);

	public TxCreateServiceTestService() throws ConfigurationException,
			ConnectionException {
		super("TxCreateService");
	}

	public void setUp() throws ConnectionException, ConfigurationException {
		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		Connection connection = connectionFactory.getConnection();
	}

	public static String serviceRequest(String args) {
		if (!JtsTransactionImple.begin())
			return "Service could not start a new transaction";

		String ior = JtsTransactionImple.getTransactionIOR();
		log.debug("TxCreateService ior: " + ior);
		return args;
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		X_OCTET rcvd = (X_OCTET) svcinfo.getBuffer();
		String rcv = new String(rcvd.getByteArray());
		String resp = serviceRequest(rcv);
		X_OCTET buffer = null;
		try {
			buffer = (X_OCTET) svcinfo.tpalloc("X_OCTET", null);
			buffer.setByteArray(resp.getBytes());
			return new Response(Connection.TPSUCCESS, 0, buffer, resp.length(),
					0);
		} catch (ConnectionException e) {
			resp = "";
			log.error("Caught an exception", e);
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
