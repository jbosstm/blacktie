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
package org.jboss.blacktie.jatmibroker.core.transport.hybrid;

import java.util.ArrayList;
import java.util.List;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.Message;
import org.jboss.blacktie.jatmibroker.core.transport.OrbManagement;
import org.jboss.blacktie.jatmibroker.core.transport.Receiver;
import org.jboss.blacktie.jatmibroker.core.tx.TxIORInterceptor;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;
import org.omg.CORBA.Object;
import org.omg.CORBA.Policy;
import org.omg.CORBA.PolicyError;
import org.omg.CosNaming.NameComponent;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.ThreadPolicyValue;
import org.omg.PortableServer.POAPackage.AdapterAlreadyExists;
import org.omg.PortableServer.POAPackage.AdapterNonExistent;

import AtmiBroker.EndpointQueuePOA;

public class CorbaReceiverImpl extends EndpointQueuePOA implements Receiver {
	private static final Logger log = LogManager
			.getLogger(CorbaReceiverImpl.class);
	private POA m_default_poa;
	private String callbackIOR;
	private List<Message> returnData = new ArrayList<Message>();
	private byte[] activate_object;
	private String queueName;
	private OrbManagement orbManagement;
	private int timeout = 0;

	private List<Policy> getPolicies(ORB orb, POA poa)
			throws ConfigurationException, ConnectionException {
		List<Policy> policies = new ArrayList<Policy>();
		Any otsPolicy = orb.create_any();

		otsPolicy.insert_short(TxIORInterceptor.ADAPTS);

		policies.add(poa
				.create_thread_policy(ThreadPolicyValue.SINGLE_THREAD_MODEL));

		try {
			policies.add(orb.create_policy(TxIORInterceptor.OTS_POLICY_TYPE,
					otsPolicy));
		} catch (PolicyError e) {
			throw new ConnectionException(-1,
					"POA TAG_OTS_POLICY policy creation error: " + e.reason, e);
		}

		return policies;
	}

	CorbaReceiverImpl(OrbManagement orbManagement, String queueName)
			throws ConnectionException {
		this.queueName = queueName;

		try {
			List<Policy> policies = getPolicies(orbManagement.getOrb(),
					orbManagement.getRootPoa());
			this.m_default_poa = orbManagement.getRootPoa().create_POA(
					queueName, orbManagement.getRootPoa().the_POAManager(),
					policies.toArray(new Policy[0]));
		} catch (Throwable t) {
			try {
				this.m_default_poa = orbManagement.getRootPoa().find_POA(
						queueName, true);
			} catch (AdapterNonExistent e) {
				throw new ConnectionException(-1, "Could not find POA:"
						+ queueName, e);
			}
		}
		try {
			activate_object = m_default_poa.activate_object(this);
			Object servant_to_reference = m_default_poa
					.servant_to_reference(this);
			NameComponent[] name = orbManagement.getNamingContextExt().to_name(
					queueName);
			orbManagement.getNamingContext().bind(name, servant_to_reference);
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Could not bind service factory"
					+ queueName, t);
		}
		this.orbManagement = orbManagement;
	}

	CorbaReceiverImpl(OrbManagement orbManagement) throws ConnectionException {
		ORB orb = orbManagement.getOrb();
		POA poa = orbManagement.getRootPoa();
		log.debug("ClientCallbackImpl constructor");

		try {
			try {
				List<Policy> policies = getPolicies(orb, poa);
				m_default_poa = poa.create_POA("TODO", poa.the_POAManager(),
						policies.toArray(new Policy[0]));
			} catch (AdapterAlreadyExists e) {
				m_default_poa = poa.find_POA("TODO", true);
			}
			log.debug("JABSession createCallbackObject");
			activate_object = m_default_poa.activate_object(this);
			log.debug("activated this " + this);

			org.omg.CORBA.Object tmp_ref = m_default_poa
					.servant_to_reference(this);
			log.debug("created reference " + tmp_ref);
			AtmiBroker.EndpointQueue clientCallback = AtmiBroker.EndpointQueueHelper
					.narrow(tmp_ref);
			log.debug("narrowed reference " + clientCallback);
			callbackIOR = orb.object_to_string(clientCallback);
			log.debug("Created:" + callbackIOR);
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Cannot create the receiver", t);
		}
		timeout = 10000; // TODO Make configurable
	}

	public POA _default_POA() {
		log.debug("ClientCallbackImpl _default_POA");
		return m_default_poa;
	}

	// client_callback() -- Implements IDL operation
	// "AtmiBroker.ClientCallback.client_callback".
	//
	public void send(String replyto_ior, short rval, int rcode, byte[] idata,
			int ilen, int cd, int flags) {
		if (callbackIOR != null) {
			log.debug("Received: " + callbackIOR);
		}
		Message message = new Message();

		message.len = ilen - 1;
		message.data = new byte[message.len];
		System.arraycopy(idata, 0, message.data, 0, message.len);

		message.cd = cd;
		message.replyTo = replyto_ior;
		message.flags = flags;
		message.control = null;// TODO
		message.rval = rval;
		message.rcode = rcode;
		synchronized (this) {
			returnData.add(message);
			notify();
		}
	}

	public java.lang.Object getReplyTo() {
		return callbackIOR;
	}

	public Message receive(long flags) throws ConnectionException {
		synchronized (this) {
			if (returnData.isEmpty()) {
				try {
					if (callbackIOR != null) {
						log.debug("Waiting" + callbackIOR);
					}
					wait(timeout);
				} catch (InterruptedException e) {
					log.error("Caught exception", e);
				}
			}
			if (returnData.isEmpty()) {
				throw new ConnectionException(-1, "Did not receive a message");
			} else {
				return returnData.remove(0);
			}
		}
	}

	public void disconnect() {
		if (queueName != null) {
			try {
				NameComponent[] name = orbManagement.getNamingContextExt()
						.to_name(queueName);
				orbManagement.getNamingContext().unbind(name);
				queueName = null;
			} catch (Throwable t) {
				log.error("Could not unbind service factory" + queueName, t);
			}
		}
		try {
			m_default_poa.deactivate_object(activate_object);
		} catch (Throwable t) {
			log.error("Could not unbind service factory" + queueName, t);
		}
		synchronized (this) {
			notify();
		}
	}

	public void close() {
		disconnect();
	}
}
