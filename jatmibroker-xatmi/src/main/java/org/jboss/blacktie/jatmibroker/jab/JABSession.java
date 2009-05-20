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

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.Transport;
import org.jboss.blacktie.jatmibroker.core.TransportFactory;

public class JABSession {
	private static final Logger log = LogManager.getLogger(JABSession.class);
	private JABSessionAttributes jabSessionAttributes;
	private Transport connection;

	public JABSession(JABSessionAttributes aJABSessionAttributes)
			throws JABException {
		super();
		log.debug("JABSession constructor ");
		try {
			jabSessionAttributes = aJABSessionAttributes;
			connection = TransportFactory.loadConnectionFactory(
					jabSessionAttributes.getProperties()).createConnection("",
					"");
		} catch (Exception e) {
			String domain = jabSessionAttributes.getDomainName();

			throw new JABException("Error connect to domain " + domain, e);
		}
	}

	public JABSessionAttributes getJABSessionAttributes() {
		return jabSessionAttributes;
	}

	public void onReply(JABReply aJabReply) throws JABException {
		log.error("JABSession Default Handler Fired for onReply: " + aJabReply);
	}

	public void endSession() throws JABException {
		log.debug("JABSession endSession ");
		connection.close();
		// TODO cleanup orb work
		connection = null;
		jabSessionAttributes = null;
	}

	public Transport getServerProxy() {
		return connection;
	}

}
