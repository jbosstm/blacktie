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
import org.jboss.blacktie.jatmibroker.core.administration.BTServiceFactoryAdministration;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;

import AtmiBroker.ServiceFactory;
import AtmiBroker.ServiceFactoryHelper;
import AtmiBroker.ServiceInfo;

public class AtmiBrokerServiceFactoryImpl implements BTServiceFactoryAdministration {
	private static final Logger log = LogManager.getLogger(AtmiBrokerServiceFactoryImpl.class);
	private ServiceFactory serviceFactory;
	private AtmiBrokerServerImpl server;
	private String serviceFactoryName;
	private POA poa;

	public synchronized static BTServiceFactoryAdministration getAdministration(String[] args, String namingContextExt, String serviceName) throws InvalidName, NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName, AdapterInactive {
		AtmiBrokerServerImpl.ConnectToORB(args, namingContextExt);
		AtmiBrokerServiceFactoryImpl instance = new AtmiBrokerServiceFactoryImpl(null, serviceName);
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

	public AtmiBroker.Service get_service(int client_id, boolean conversation, org.omg.CORBA.StringHolder id) throws Exception {
		return serviceFactory.get_service(client_id, conversation, id);
	}

	public AtmiBroker.Service find_service(int client_id, java.lang.String id) throws Exception {
		return serviceFactory.find_service(client_id, id);
	}

	public String get_service_id(boolean conversation, org.omg.CORBA.StringHolder id) {
		log.debug("ServiceFactoryProxy's get_service_id conversation: " + conversation);

		return serviceFactory.get_service_id(server.getClientId(), conversation, id);
	}

	public void end_conversation(int client_id, String id) throws Exception {
		log.debug("ServiceFactoryProxy's end_conversation client_id: " + client_id + " id: " + id);

		serviceFactory.end_conversation(client_id, id);
	}

	public String find_service_id(int client_id, String id) throws Exception {
		log.debug("ServiceFactoryProxy's find_service_id client_id: " + client_id + " id: " + id);

		return serviceFactory.find_service_id(client_id, id);
	}

	public ServiceInfo get_service_info() throws Exception {
		log.debug("ServiceFactoryProxy's get_service_info");

		return serviceFactory.get_service_info();
	}

	public void close() {
		// TODO Auto-generated method stub

	}

}
