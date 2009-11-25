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
package org.jboss.blacktie.administration;

import java.io.IOException;
import java.util.StringTokenizer;

import javax.ejb.ActivationConfigProperty;
import javax.ejb.MessageDriven;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.administration.core.AdministrationProxy;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;
import org.jboss.blacktie.jatmibroker.xatmi.mdb.MDBBlacktieService;
import org.jboss.ejb3.annotation.Depends;

@MessageDriven(activationConfig = {
		@ActivationConfigProperty(propertyName = "destinationType", propertyValue = "javax.jms.Queue"),
		@ActivationConfigProperty(propertyName = "destination", propertyValue = "queue/BTDomainAdmin") })
@Depends("jboss.messaging.destination:service=Queue,name=BTDomainAdmin")
@javax.ejb.TransactionAttribute(javax.ejb.TransactionAttributeType.NOT_SUPPORTED)
public class BlacktieAdminServiceXATMI extends MDBBlacktieService implements
		javax.jms.MessageListener {
	private static final Logger log = LogManager
			.getLogger(BlacktieAdminServiceXATMI.class);

	private AdministrationProxy administrationProxy;

	public BlacktieAdminServiceXATMI() throws IOException,
			ConfigurationException, ConnectionException {
		super("BTDomainAdmin");
		administrationProxy = new AdministrationProxy();
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		log.debug("Message received");
		X_OCTET recv = (X_OCTET) svcinfo.getBuffer();
		String string = new String(recv.getByteArray());
		StringTokenizer st = new StringTokenizer(string, ",", false);
		String operation = st.nextToken();
		byte[] toReturn = new byte[1];
		try {
			if (operation.equals("shutdown")) {
				log.trace("shutdown");
				String serverName = st.nextToken();
				int id = 0;
				if (st.hasMoreTokens()) {
					id = Integer.parseInt(st.nextToken());
				}
				administrationProxy.shutdown(serverName, id);
				toReturn[0] = 1;
			}
			X_OCTET buffer = (X_OCTET) svcinfo.tpalloc("X_OCTET", null);
			buffer.setByteArray(toReturn);
			log.debug("Responding");
			return new Response(Connection.TPSUCCESS, 0, buffer, 1, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
