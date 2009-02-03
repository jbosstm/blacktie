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
import org.omg.PortableServer.LifespanPolicyValue;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.AdapterNonExistent;

import AtmiBroker.ServiceInfo;
import AtmiBroker.ServiceQueuePOA;

public class ServiceQueue extends ServiceQueuePOA {
	private static final Logger log = LogManager.getLogger(ServiceQueue.class);
	private String serviceName;
	private POA poa;
	private AtmiBroker.ServiceQueue me;
	private byte[] activate_object;
	private List<AtmiBroker_ServiceImpl> servantCache = new ArrayList<AtmiBroker_ServiceImpl>();
	private List<Message> messageQueue = new ArrayList<Message>();

	ServiceQueue(String serviceName, int servantCacheSize, Class atmiBrokerCallback, AtmiBroker_CallbackConverter atmiBroker_CallbackConverter) throws JAtmiBrokerException, InstantiationException, IllegalAccessException {
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

		for (int i = 0; i < servantCacheSize; i++) {
			servantCache.add(new AtmiBroker_ServiceImpl(serviceName, atmiBrokerCallback, atmiBroker_CallbackConverter, this));
		}
	}

	public void bind() throws JAtmiBrokerException {
		try {
			activate_object = poa.activate_object(this);
			Object servant_to_reference = poa.servant_to_reference(this);
			NameComponent[] name = AtmiBrokerServerImpl.nce.to_name(serviceName);
			AtmiBrokerServerImpl.nc.bind(name, servant_to_reference);
			me = AtmiBroker.ServiceQueueHelper.narrow(servant_to_reference);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not bind service factory" + serviceName, t);
		}
	}

	public void unbind() throws JAtmiBrokerException {
		try {
			NameComponent[] name = AtmiBrokerServerImpl.nce.to_name(serviceName);
			AtmiBrokerServerImpl.nc.unbind(name);
			poa.deactivate_object(activate_object);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not unbind service factory" + serviceName, t);
		}
	}

	public void send(String replyto_ior, byte[] idata, int ilen, int flags) {
		Message message = new Message();
		message.replyTo = replyto_ior;
		message.data = idata;
		message.len = ilen;
		message.flags = flags;
		messageQueue.add(message);
	}

	public ServiceInfo get_service_info() {
		log.error("NO-OP get_service_info");
		// TODO Auto-generated method stub
		return null;
	}

	public Message receive(long flags) {
		synchronized (this) {
			while (messageQueue.isEmpty()) {
				try {
					wait();
				} catch (InterruptedException e) {
					log.error("Caught exception", e);
				}
			}
			if (messageQueue.isEmpty()) {
				return null;
			} else {
				return messageQueue.remove(0);
			}
		}
	}
}
