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

import java.util.ArrayList;
import java.util.List;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.proxy.Queue;
import org.omg.CORBA.Policy;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.AdapterAlreadyExists;
import org.omg.PortableServer.POAPackage.AdapterNonExistent;
import org.omg.PortableServer.POAPackage.InvalidPolicy;
import org.omg.PortableServer.POAPackage.ServantAlreadyActive;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import AtmiBroker.EndpointQueuePOA;

public class EndpointQueue extends EndpointQueuePOA implements Queue {
	private static final Logger log = LogManager.getLogger(EndpointQueue.class);
	private POA m_default_poa;
	private String callbackIOR;
	private List<Message> returnData = new ArrayList<Message>();

	public EndpointQueue(POA poa, String aServerName) throws AdapterNonExistent, InvalidPolicy, ServantAlreadyActive, WrongPolicy, ServantNotActive {
		super();
		log.debug("ClientCallbackImpl constructor ");
		int numberOfPolicies = 0;
		Policy[] policiesArray = new Policy[numberOfPolicies];
		List<Policy> policies = new ArrayList<Policy>();
		// policies.add(AtmiBrokerServerImpl.root_poa.create_thread_policy(ThreadPolicyValue.SINGLE_THREAD_MODEL));
		policies.toArray(policiesArray);

		try {
			m_default_poa = poa.create_POA(aServerName, poa.the_POAManager(), policiesArray);
		} catch (AdapterAlreadyExists e) {
			m_default_poa = poa.find_POA(aServerName, true);
		}
		log.debug("JABSession createCallbackObject ");
		m_default_poa.activate_object(this);
		log.debug("activated this " + this);

		org.omg.CORBA.Object tmp_ref = m_default_poa.servant_to_reference(this);
		log.debug("created reference " + tmp_ref);
		AtmiBroker.EndpointQueue clientCallback = AtmiBroker.EndpointQueueHelper.narrow(tmp_ref);
		log.debug("narrowed reference " + clientCallback);
		callbackIOR = AtmiBrokerServerImpl.orb.object_to_string(clientCallback);
		log.debug(" created ClientCallback ior " + callbackIOR);
	}

	public POA _default_POA() {
		log.debug("ClientCallbackImpl _default_POA");
		return m_default_poa;
	}

	// client_callback() -- Implements IDL operation
	// "AtmiBroker.ClientCallback.client_callback".
	//
	public void send(String replyto_ior, short rval, int rcode, byte[] idata, int ilen, int flags, int revent) {
		log.error("Default client_callback called");
		log.debug("client_callback(): called.");
		log.debug("    idata = " + new String(idata));
		log.debug("    ilen = " + ilen);
		log.debug("    flags = " + flags);
		log.debug("client_callback(): returning.");
		Message message = new Message();
		message.replyTo = replyto_ior;
		message.data = idata;
		message.len = ilen;
		message.flags = flags;
		message.control = null;// TODO
		message.rval = rval;
		message.rcode = rcode;
		message.event = revent;
		returnData.add(message);
	}

	public String getReplyTo() {
		return callbackIOR;
	}

	public Message receive(long flags) {
		// TODO
		Message toReturn = returnData.remove(0);
		return toReturn;
	}

	public void disconnect() {
		// TODO Auto-generated method stub

	}

	public void close() {
		// TODO Auto-generated method stub

	}
}
