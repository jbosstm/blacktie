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
import java.util.Iterator;
import java.util.List;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.omg.CORBA.Object;
import org.omg.CORBA.Policy;
import org.omg.CORBA.StringHolder;
import org.omg.CosNaming.NameComponent;
import org.omg.PortableServer.LifespanPolicyValue;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.AdapterNonExistent;

import AtmiBroker.ServiceFactory;
import AtmiBroker.ServiceFactoryPOA;
import AtmiBroker.ServiceInfo;

public class AtmiBroker_ServiceFactoryImpl extends ServiceFactoryPOA {
	private static final Logger log = LogManager.getLogger(AtmiBroker_ServiceFactoryImpl.class);
	private String serviceName;
	private POA poa;
	private ServiceFactory me;
	private byte[] activate_object;
	private List<AtmiBroker_ServiceImpl> servantCache = new ArrayList<AtmiBroker_ServiceImpl>();

	AtmiBroker_ServiceFactoryImpl(String serviceName, int servantCacheSize, Class atmiBrokerCallback, AtmiBroker_CallbackConverter atmiBroker_Callback) throws JAtmiBrokerException {
		this.serviceName = serviceName;
		int numberOfPolicies = 1;
		Policy[] policiesArray = new Policy[numberOfPolicies];
		List<Policy> policies = new ArrayList<Policy>();
		policies.add(AtmiBrokerServerImpl.root_poa.create_lifespan_policy(LifespanPolicyValue.PERSISTENT));
		// policies.add(AtmiBrokerServerImpl.root_poa.create_thread_policy(ThreadPolicyValue.SINGLE_THREAD_MODEL));
		policies.toArray(policiesArray);

		try {
			this.poa = AtmiBrokerServerImpl.root_poa.create_POA(serviceName, AtmiBrokerServerImpl.root_poa.the_POAManager(), policiesArray);
		} catch (Throwable t) {
			try {
				this.poa = AtmiBrokerServerImpl.root_poa.find_POA(serviceName, true);
			} catch (AdapterNonExistent e) {
				throw new JAtmiBrokerException("Could not find POA:" + serviceName, e);
			}
		}

		createCache(servantCacheSize, atmiBrokerCallback, atmiBroker_Callback);
	}

	public AtmiBroker_ServiceImpl createAtmiBroker_ServiceImpl(String serviceName, Class callback, AtmiBroker_CallbackConverter atmiBroker_CallbackConverter) throws JAtmiBrokerException {
		try {
			return new AtmiBroker_ServiceImpl(serviceName, callback, atmiBroker_CallbackConverter);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not create AtmiBroker_ServiceImpl: " + serviceName, t);
		}
	}

	public void bind() throws JAtmiBrokerException {
		try {
			activate_object = poa.activate_object(this);
			Object servant_to_reference = poa.servant_to_reference(this);
			NameComponent[] name = AtmiBrokerServerImpl.nce.to_name(serviceName);
			AtmiBrokerServerImpl.nc.rebind(name, servant_to_reference);
			me = AtmiBroker.ServiceFactoryHelper.narrow(servant_to_reference);

			Iterator<AtmiBroker_ServiceImpl> iterator = servantCache.iterator();
			while (iterator.hasNext()) {
				iterator.next().bind();
			}
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not bind service factory" + serviceName, t);
		}
	}

	public void unbind() throws JAtmiBrokerException {
		try {
			Iterator<AtmiBroker_ServiceImpl> iterator = servantCache.iterator();
			while (iterator.hasNext()) {
				iterator.next().unbind();
			}

			poa.deactivate_object(activate_object);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not unbind service factory" + serviceName, t);
		}
	}

	public void createCache(int servantCacheSize, Class atmiBroker_Callback, AtmiBroker_CallbackConverter atmiBroker_CallbackConverter) throws JAtmiBrokerException {
		for (int i = 0; i < servantCacheSize; i++) {
			servantCache.add(createAtmiBroker_ServiceImpl(serviceName, atmiBroker_Callback, atmiBroker_CallbackConverter));
		}
	}

	public void end_conversation(String id) {
		log.error("NO-OP end_conversation");
		// TODO Auto-generated method stub

	}

	public String start_conversation(StringHolder id) {
		log.error("NO-OP get_service_id");
		// TODO Auto-generated method stub
		id.value = serviceName + ":" + 0;
		String ior = servantCache.get(0).getIor();
		log.debug("Returning servant IOR: " + ior);
		return ior;
	}

	public ServiceInfo get_service_info() {
		log.error("NO-OP get_service_info");
		// TODO Auto-generated method stub
		return null;
	}

}
