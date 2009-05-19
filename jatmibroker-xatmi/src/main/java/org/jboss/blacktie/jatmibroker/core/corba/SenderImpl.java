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
package org.jboss.blacktie.jatmibroker.core.corba;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.AtmiBrokerServerProxy;
import org.jboss.blacktie.jatmibroker.core.OrbManagement;
import org.jboss.blacktie.jatmibroker.core.proxy.Sender;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;

import AtmiBroker.EndpointQueue;
import AtmiBroker.EndpointQueueHelper;

public class SenderImpl implements Sender {
	private static final Logger log = LogManager.getLogger(SenderImpl.class);
	private EndpointQueue serviceFactory;

	public synchronized static Sender createSender(
			AtmiBrokerServerProxy server, String serviceName) throws NotFound,
			CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName {
		org.omg.CORBA.Object serviceFactoryObject = server.getOrbManagement()
				.getNamingContext().resolve(
						server.getOrbManagement().getNamingContextExt()
								.to_name(serviceName));
		SenderImpl instance = new SenderImpl(serviceFactoryObject, serviceName);
		return instance;
	}

	public synchronized static Sender createSender(OrbManagement orbManagement,
			String callback_ior) {
		org.omg.CORBA.Object serviceFactoryObject = orbManagement.getOrb()
				.string_to_object(callback_ior);
		SenderImpl instance = new SenderImpl(serviceFactoryObject, callback_ior);
		return instance;
	}

	protected SenderImpl(org.omg.CORBA.Object serviceFactoryObject,
			String serviceFactoryName) {
		serviceFactory = EndpointQueueHelper.narrow(serviceFactoryObject);
		log.debug("ServiceFactory is " + serviceFactory);
	}

	public void send(String replyTo, short rval, int rcode, byte[] data,
			int len, int correlationId, int flags) {
		serviceFactory.send(replyTo, rval, rcode, data, len, correlationId,
				flags);
	}

	public void close() {
		// TODO Auto-generated method stub
	}
}
