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
package org.jboss.blacktie.jatmibroker.core;

import java.util.ArrayList;
import java.util.List;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.omg.CORBA.Object;
import org.omg.CORBA.Policy;
import org.omg.CosNaming.NameComponent;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.CosTransactions.Control;
import org.omg.PortableServer.LifespanPolicyValue;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.AdapterNonExistent;
import org.omg.PortableServer.POAPackage.ServantAlreadyActive;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import AtmiBroker.Service;
import AtmiBroker.ServiceManager;
import AtmiBroker.ServiceManagerPOA;

public class AtmiBroker_ServiceManagerImpl extends ServiceManagerPOA {
	private static final Logger log = LogManager.getLogger(AtmiBroker_ServiceManagerImpl.class);
	private POA poa;
	private ServiceManager me;
	private String serviceManagerName;
	private byte[] activate_object;

	AtmiBroker_ServiceManagerImpl(String service) throws AdapterNonExistent, ServantAlreadyActive, WrongPolicy, ServantNotActive, NotFound, CannotProceed, InvalidName {
		serviceManagerName = service + "Manager";
		int numberOfPolicies = 1;
		Policy[] policiesArray = new Policy[numberOfPolicies];
		List<Policy> policies = new ArrayList<Policy>();
		policies.add(AtmiBrokerServerImpl.root_poa.create_lifespan_policy(LifespanPolicyValue.PERSISTENT));
		// policies.add(AtmiBrokerServerImpl.root_poa.create_thread_policy(ThreadPolicyValue.SINGLE_THREAD_MODEL));
		policies.toArray(policiesArray);

		try {
			this.poa = AtmiBrokerServerImpl.root_poa.create_POA(serviceManagerName, AtmiBrokerServerImpl.root_poa.the_POAManager(), policiesArray);
		} catch (Throwable t) {
			this.poa = AtmiBrokerServerImpl.root_poa.find_POA(serviceManagerName, true);
		}
	}

	public void bind() throws JAtmiBrokerException {
		try {
			activate_object = poa.activate_object(this);
			Object servant_to_reference = poa.servant_to_reference(this);
			NameComponent[] name = AtmiBrokerServerImpl.nce.to_name(serviceManagerName);
			AtmiBrokerServerImpl.nc.rebind(name, servant_to_reference);
			me = AtmiBroker.ServiceManagerHelper.narrow(servant_to_reference);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not bind service manager: " + serviceManagerName, t);
		}
	}

	public void unbind() throws JAtmiBrokerException {
		try {
			poa.deactivate_object(activate_object);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not unbind service manager: " + serviceManagerName, t);
		}
	}

	public String serviceName() {
		log.error("NO-OP serviceName");
		// TODO Auto-generated method stub
		return null;
	}

	public void send_data(String ior, boolean inConversation, byte[] idata, int ilen, int flags, int revent) {
		Object tmp_ref = AtmiBrokerServerImpl.orb.string_to_object(ior);
		Service aItrPtr = AtmiBroker.ServiceHelper.narrow(tmp_ref);
		aItrPtr.send_data(inConversation, idata, ilen, flags, revent);
	}
}
