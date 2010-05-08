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
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.example.ejb.eg1.BTTestRemote;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.JtsTransactionImple;
import org.jboss.blacktie.jatmibroker.jab.JABException;
import org.jboss.blacktie.jatmibroker.jab.JABTransaction;
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
		@ActivationConfigProperty(propertyName = "destination", propertyValue = "queue/TxEchoService") })
@Depends("jboss.messaging.destination:service=Queue,name=TxEchoService")
public class TxEchoServiceTestService extends MDBBlacktieService implements
		javax.jms.MessageListener {
	private static final Logger log = LogManager
			.getLogger(TxEchoServiceTestService.class);
	private static final String[] names = { "FirstBTBean/remote",
			"SecondBTBean/remote" };

	public TxEchoServiceTestService() throws ConnectionException,
			ConfigurationException {
		super("TxEchoService");
	}

	public static Connection getConnection() throws ConnectionException,
			ConfigurationException {
		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		return connectionFactory.getConnection();
	}

	public static String serviceRequest(String args) throws NamingException {
		Context ctx = new InitialContext();
		Object[] objs = new Object[names.length];
		BTTestRemote[] beans = new BTTestRemote[names.length];
		String[] results = new String[names.length];

		for (int i = 0; i < names.length; i++) {
			objs[i] = ctx.lookup(names[i]);
			beans[i] = (BTTestRemote) objs[i];
			// results[i] = beans[i].echo("bean=" + names[(i + 1) %
			// names.length]);
			// log.debug(names[i] + " result: " + results[i]);
		}

		log.debug(args + " hasTransaction: "
				+ JtsTransactionImple.hasTransaction());

		if (args.contains("tx=true")) {
			try {
				String s = beans[0].txNever("bean=" + names[1]);
				log.info("Error should have got a Not Supported Exception");
				return "Error should have got a Not Supported Exception";
			} catch (javax.ejb.EJBException e) {
				log.debug("Success got Exception calling txNever: " + e);
				return args;
			}
		} else if (args.contains("tx=false")) {
			try {
				String s = beans[0].txMandatory("bean=" + names[1]);
				log
						.info("Error should have got an EJBTransactionRequiredException exception");
				return "Error should have got an EJBTransactionRequiredException exception";
			} catch (javax.ejb.EJBTransactionRequiredException e) {
				log.debug("Success got EJBTransactionRequiredException");
				return args;
			}
		} else if (args.contains("tx=create")) {
			try {
				Connection connection = getConnection();
				byte[] echo = args.getBytes();
				X_OCTET buffer = (X_OCTET) connection.tpalloc("X_OCTET", null);
				buffer.setByteArray(echo);

				log.debug("Invoking TxCreateService...");
				Response response = connection.tpcall("TxCreateService",
						buffer, echo.length, 0);
				X_OCTET rcvd = (X_OCTET) response.getBuffer();
				String responseData = new String(rcvd.getByteArray());
				log.debug("TxCreateService response: " + responseData);

				// check that the remote service created a transaction
				JABTransaction tx = JABTransaction.current();
				if (tx != null) {
					try {
						tx.commit();
					} catch (JABException e) {
						args = "Service create a transaction but commit failed: "
								+ e;
					}
				} else {
					args = "Service should have propagated a new transaction back to caller";
				}

				return responseData; // should be the same as args
			} catch (ConnectionException e) {
				return e.getMessage();
			} catch (ConfigurationException e) {
				return e.getMessage();
			}
			// if (!JtsTransactionImple.begin())
			// return "Service could not start a new transaction";
			// String ior = JtsTransactionImple.getTransactionIOR();
			// return args;
		} else {
			try {
				return beans[0].echo("bean=" + names[1]);
			} catch (javax.ejb.EJBException e) {
				log
						.warn("Failure got Exception calling method with default transaction attribute");
				return "Failure got Exception calling method with default transaction attribute: "
						+ e;
			}
		}
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		X_OCTET rcv = (X_OCTET) svcinfo.getBuffer();
		String rcvd = new String(rcv.getByteArray());
		String resp;
		try {
			resp = serviceRequest(new String(rcvd));
		} catch (javax.naming.NamingException e) {
			log.warn("error: " + e);
			resp = e.getMessage();
		}
		try {
			X_OCTET buffer = (X_OCTET) svcinfo.getConnection().tpalloc(
					"X_OCTET", null);
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
