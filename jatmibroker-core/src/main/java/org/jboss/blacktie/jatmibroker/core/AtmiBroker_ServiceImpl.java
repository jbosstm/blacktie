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
import org.omg.PortableServer.POA;
import org.omg.PortableServer.ThreadPolicyValue;
import org.omg.PortableServer.POAPackage.AdapterNonExistent;
import org.omg.PortableServer.POAPackage.ServantAlreadyActive;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import AtmiBroker.Service;
import AtmiBroker.ServicePOA;

public class AtmiBroker_ServiceImpl extends ServicePOA {
	private static final Logger log = LogManager.getLogger(AtmiBroker_ServiceImpl.class);
	private java.lang.Object callback;
	private String serviceName;
	private POA poa;
	private Service me;
	private byte[] activate_object;
	private java.lang.Object toCall;
	private String ior;
	private AtmiBroker_CallbackConverter atmiBroker_CallbackConverter;

	AtmiBroker_ServiceImpl(String serviceName, Class callback, AtmiBroker_CallbackConverter atmiBroker_CallbackConverter) throws AdapterNonExistent, ServantAlreadyActive, WrongPolicy, ServantNotActive, InstantiationException, IllegalAccessException {
		this.serviceName = serviceName;
		this.callback = callback.newInstance();
		this.atmiBroker_CallbackConverter = atmiBroker_CallbackConverter;
		Policy[] policiesArray = new Policy[1];
		List<Policy> policies = new ArrayList<Policy>(1);
		policies.add(AtmiBrokerServerImpl.root_poa.create_thread_policy(ThreadPolicyValue.ORB_CTRL_MODEL));
		policies.toArray(policiesArray);
		try {
			this.poa = AtmiBrokerServerImpl.root_poa.create_POA(serviceName, AtmiBrokerServerImpl.root_poa.the_POAManager(), policiesArray);
		} catch (Throwable t) {
			this.poa = AtmiBrokerServerImpl.root_poa.find_POA(serviceName, true);
		}
	}

	public void bind() throws JAtmiBrokerException {
		try {
			activate_object = poa.activate_object(this);
			byte[] servant_to_id = poa.servant_to_id(this);
			Object create_reference_with_id = poa.create_reference_with_id(servant_to_id, "IDL:AtmiBroker/Service:1.0");
			String reference = create_reference_with_id.toString();
			me = AtmiBroker.ServiceHelper.narrow(create_reference_with_id);
			ior = AtmiBrokerServerImpl.orb.object_to_string(me);
			log.debug("My IOR is: " + ior);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not bind service: " + serviceName, t);
		}
	}

	public void unbind() throws JAtmiBrokerException {
		try {
			poa.deactivate_object(activate_object);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not unbind service: " + serviceName, t);
		}
	}

	public String getIor() {
		return ior;
	}

	public void send_data(boolean inConversation, String callbackIOR, byte[] idata, int ilen, int flags, int revent) {
		// TODO HANDLE CONTROL
		try {
			// THIS IS THE FIRST CALL
			if (!inConversation) {
				AtmiBroker_Response serviceResponse = atmiBroker_CallbackConverter.serviceRequest(callback, serviceName, idata, ilen, flags);
			}
			// TODO THIS SHOULD INVOKE THE CLIENT HANDLER
			// odata.value = serviceRequest.getBytes();
			// olen.value = serviceRequest.getLength();
		} catch (Throwable t) {
			log.error("Could not service the request");
		}
	}

	public String serviceName() {
		return serviceName;
	}
}
