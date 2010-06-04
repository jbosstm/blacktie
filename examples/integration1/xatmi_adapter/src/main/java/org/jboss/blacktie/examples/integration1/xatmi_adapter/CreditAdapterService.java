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
package org.jboss.blacktie.examples.integration1.xatmi_adapter;

import javax.ejb.ActivationConfigProperty;
import javax.ejb.MessageDriven;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.examples.integration1.ejb.CreditRemote;
import org.jboss.blacktie.jatmibroker.core.transport.JtsTransactionImple;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_COMMON;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;
import org.jboss.blacktie.jatmibroker.xatmi.mdb.MDBBlacktieService;
import org.jboss.ejb3.annotation.Depends;

@javax.ejb.TransactionAttribute(javax.ejb.TransactionAttributeType.NOT_SUPPORTED)
@MessageDriven(activationConfig = {
		@ActivationConfigProperty(propertyName = "destinationType", propertyValue = "javax.jms.Queue"),
		@ActivationConfigProperty(propertyName = "destination", propertyValue = "queue/CREDIT") })
@Depends("jboss.messaging.destination:service=Queue,name=CREDIT")
public class CreditAdapterService extends MDBBlacktieService implements
		javax.jms.MessageListener {
	private static final Logger log = LogManager
			.getLogger(CreditAdapterService.class);

	public CreditAdapterService() {
		super("CREDIT");
	}

	public Response tpservice(TPSVCINFO svcinfo) throws ConnectionException {
		log.info(" hasTransaction: " + JtsTransactionImple.hasTransaction());

		X_COMMON rcv = (X_COMMON) svcinfo.getBuffer();
		long acct_no = rcv.getLong("acct_no");
		short amount = rcv.getShort("amount");

		String resp = "NAMINGERROR";
		try {
			Context ctx = new InitialContext();
			CreditRemote bean = (CreditRemote) ctx.lookup("CreditBean/remote");
			log.debug("resolved CreditBean");
			resp = bean.credit(acct_no, amount);
		} catch (NamingException e) {
			log.error("Got a naming error: " + e.getMessage(), e);
		}
		log.info("Returning: " + resp);

		/*
		 * try { String s = beans[0].txNever("bean=" + names[1]);
		 * log.info("Error should have got a Not Supported Exception"); return
		 * "Error should have got a Not Supported Exception"; } catch
		 * (javax.ejb.EJBException e) {
		 * log.debug("Success got Exception calling txNever: " + e); return
		 * args; }
		 */
		X_OCTET buffer = (X_OCTET) svcinfo.getConnection().tpalloc("X_OCTET",
				null, resp.length() + 1);
		buffer.setByteArray(resp.getBytes());
		return new Response(Connection.TPSUCCESS, 0, buffer, 0);
	}
}
