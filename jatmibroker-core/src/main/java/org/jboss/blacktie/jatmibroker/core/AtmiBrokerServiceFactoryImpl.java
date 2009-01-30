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
import org.jboss.blacktie.jatmibroker.core.proxy.AtmiBrokerServiceFactory;
import org.omg.CORBA.Object;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.PortableServer.POA;

import AtmiBroker.Service;
import AtmiBroker.ServiceFactory;
import AtmiBroker.ServiceFactoryHelper;

public class AtmiBrokerServiceFactoryImpl implements AtmiBrokerServiceFactory {
	private static final Logger log = LogManager.getLogger(AtmiBrokerServiceFactoryImpl.class);
	private ServiceFactory serviceFactory;
	private AtmiBrokerServerImpl server;
	private String serviceFactoryName;
	private POA poa;

	public synchronized static AtmiBrokerServiceFactory getProxy(AtmiBrokerServerImpl server, String serviceName) throws NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName {
		AtmiBrokerServiceFactoryImpl instance = new AtmiBrokerServiceFactoryImpl(server, serviceName);
		return instance;
	}

	protected AtmiBrokerServiceFactoryImpl(AtmiBrokerServerImpl server, String serviceFactoryName) throws NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName {
		this.server = server;
		this.serviceFactoryName = serviceFactoryName;
		log.debug("ServiceFactoryProxy's ServiceFactoryName: " + serviceFactoryName);
		org.omg.CORBA.Object serviceFactoryObject = AtmiBrokerServerImpl.nc.resolve(AtmiBrokerServerImpl.nce.to_name(serviceFactoryName));
		log.debug("ServiceFactory Object is " + serviceFactoryObject);
		log.debug("ServiceFactory class is " + serviceFactoryObject.getClass().getName());
		serviceFactory = ServiceFactoryHelper.narrow(serviceFactoryObject);
		log.debug("ServiceFactory is " + serviceFactory);
	}

	public String start_conversation(org.omg.CORBA.StringHolder id) throws JAtmiBrokerException {
		log.debug("ServiceFactoryProxy's get_service_id");

		return serviceFactory.start_conversation(id);
	}

	public void end_conversation(String id) throws JAtmiBrokerException {
		log.debug("ServiceFactoryProxy's end_conversation id: " + id);

		serviceFactory.end_conversation(id);
	}

	public void close() {
		// TODO Auto-generated method stub

	}

	public void send_data(String ior, byte[] data, int flags) {
		Object tmp_ref = AtmiBrokerServerImpl.orb.string_to_object(ior);
		Service service = AtmiBroker.ServiceHelper.narrow(tmp_ref);
		service.send_data(false, server.getClientCallbackIOR(), data, data.length, flags, 0);
	}

}
