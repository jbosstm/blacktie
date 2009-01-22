/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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
/*
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT
package org.jboss.blacktie.jatmibroker.core;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.administration.BTServiceManagerAdministration;
import org.jboss.blacktie.jatmibroker.core.proxy.AtmiBrokerServiceManager;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.CosTransactions.Control;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;

import AtmiBroker.ServiceManager;
import AtmiBroker.ServiceManagerHelper;

public class AtmiBrokerServiceManagerImpl implements BTServiceManagerAdministration, AtmiBrokerServiceManager {
	private static final Logger log = LogManager.getLogger(AtmiBrokerServiceManagerImpl.class);
	private ServiceManager serviceManager;
	private org.omg.CORBA.Object serviceManagerObject;
	private AtmiBrokerServiceFactoryImpl serviceFactoryImpl;

	static AtmiBrokerServiceManager getProxy(AtmiBrokerServerImpl server, String serviceManagerName) throws InvalidName, NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName, AdapterInactive {
		AtmiBrokerServiceManagerImpl instance = new AtmiBrokerServiceManagerImpl(server, serviceManagerName);
		return instance;
	}

	public static BTServiceManagerAdministration getAdministration(String[] args, String namingContextExt, String serviceManagerName) throws InvalidName, NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName, AdapterInactive {
		AtmiBrokerServerImpl.ConnectToORB(args, namingContextExt);
		AtmiBrokerServiceManagerImpl instance = new AtmiBrokerServiceManagerImpl(null, serviceManagerName);
		return instance;
	}

	protected AtmiBrokerServiceManagerImpl(AtmiBrokerServerImpl server, String serviceName) throws NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName {
		this.serviceFactoryImpl = new AtmiBrokerServiceFactoryImpl(server, serviceName);
		serviceManager = null;
		String serviceManagerName = serviceName + "Manager";
		log.debug("ServiceManagerProxy's ServiceManagerName: " + serviceManagerName);

		serviceManagerObject = AtmiBrokerServerImpl.nc.resolve(AtmiBrokerServerImpl.nce.to_name(serviceManagerName));
		log.debug("ServiceManager Object is " + serviceManagerObject);
		log.debug("ServiceManager class is " + serviceManagerObject.getClass().getName());

		serviceManager = ServiceManagerHelper.narrow(serviceManagerObject);
		log.debug("ServiceManager is " + serviceManager);
	}

	public String serviceName() throws Exception {
		log.debug("ServiceManagerProxy's serviceName ");

		return serviceManager.serviceName();
	}

	public void send_data(String ior, boolean inConversation, byte[] idata, int ilen, int flags, int revent) throws JAtmiBrokerException {
		log.debug("ServiceManagerProxy's service_request_async ior: " + ior + " idata: " + idata + " ilen: " + ilen + " inConversation: " + inConversation + " flags: " + flags);

		serviceManager.send_data(ior, inConversation, idata, ilen, flags, revent);
	}

	public void close() {
		// TODO Auto-generated method stub

	}
}
