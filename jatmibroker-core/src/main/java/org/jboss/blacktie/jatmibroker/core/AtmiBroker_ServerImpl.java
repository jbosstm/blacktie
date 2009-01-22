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
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.omg.CORBA.Object;
import org.omg.CORBA.Policy;
import org.omg.CosNaming.NameComponent;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.ThreadPolicyValue;

import AtmiBroker.ClientInfo;
import AtmiBroker.EnvVariableInfo;
import AtmiBroker.Server;
import AtmiBroker.ServerInfo;
import AtmiBroker.ServerPOA;
import AtmiBroker.ServiceInfo;

public class AtmiBroker_ServerImpl extends ServerPOA {
	private static final Logger log = LogManager.getLogger(AtmiBroker_ServerImpl.class);
	private POA poa;
	private Server me;
	private String serverName;
	private byte[] activate_object;
	private Map<String, AtmiBroker_ServiceFactoryImpl> serviceFactoryList = new HashMap<String, AtmiBroker_ServiceFactoryImpl>();
	private boolean bound;

	public AtmiBroker_ServerImpl(Properties properties) throws JAtmiBrokerException {
		String companyName = properties.getProperty("blacktie.company.name");
		String serverName = properties.getProperty("blacktie.server.name");
		int numberOfOrbArgs = Integer.parseInt(properties.getProperty("blacktie.orb.args"));
		List<String> orbArgs = new ArrayList<String>(numberOfOrbArgs);
		for (int i = 1; i <= numberOfOrbArgs; i++) {
			orbArgs.add(properties.getProperty("blacktie.orb.arg." + i));
		}
		String[] args = orbArgs.toArray(new String[] {});
		try {
			AtmiBrokerServerImpl.ConnectToORB(args, companyName);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not connect to orb for company name: " + companyName, t);
		}
		this.serverName = serverName;
		Policy[] policiesArray = new Policy[1];
		List<Policy> policies = new ArrayList<Policy>(1);
		policies.add(AtmiBrokerServerImpl.root_poa.create_thread_policy(ThreadPolicyValue.ORB_CTRL_MODEL));
		policies.toArray(policiesArray);
		try {
			this.poa = AtmiBrokerServerImpl.root_poa.create_POA(serverName, AtmiBrokerServerImpl.root_poa.the_POAManager(), policiesArray);
		} catch (Throwable t) {
			try {
				this.poa = AtmiBrokerServerImpl.root_poa.find_POA(serverName, true);
			} catch (Throwable t2) {
				throw new JAtmiBrokerException("Could not find poa", t2);
			}
		}
	}

	public void createAtmiBroker_ServiceFactoryImpl(String serviceName, int servantCacheSize, Class callback, AtmiBroker_CallbackConverter atmiBroker_Callback) throws JAtmiBrokerException {
		if (!serviceFactoryList.containsKey(serviceName)) {
			try {
				AtmiBroker_ServiceFactoryImpl atmiBroker_ServiceFactoryImpl = new AtmiBroker_ServiceFactoryImpl(serviceName, servantCacheSize, callback, atmiBroker_Callback);
				serviceFactoryList.put(serviceName, atmiBroker_ServiceFactoryImpl);
				if (bound) {
					atmiBroker_ServiceFactoryImpl.bind();
				}
			} catch (Throwable t) {
				throw new JAtmiBrokerException("Could not create service factory for: " + serviceName, t);
			}
		}
	}

	public void bind() throws JAtmiBrokerException {
		try {
			activate_object = poa.activate_object(this);
			NameComponent[] name = AtmiBrokerServerImpl.nce.to_name(serverName);
			Object servant_to_reference = poa.servant_to_reference(this);
			AtmiBrokerServerImpl.nc.rebind(name, servant_to_reference);
			me = AtmiBroker.ServerHelper.narrow(servant_to_reference);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not bind server", t);
		}
		Iterator<AtmiBroker_ServiceFactoryImpl> iterator = serviceFactoryList.values().iterator();
		while (iterator.hasNext()) {
			iterator.next().bind();
		}
		bound = true;
	}

	public void unbind() throws JAtmiBrokerException {
		Iterator<AtmiBroker_ServiceFactoryImpl> iterator = serviceFactoryList.values().iterator();
		while (iterator.hasNext()) {
			iterator.next().unbind();
		}
		try {
			poa.deactivate_object(activate_object);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not unbind server", t);
		}
		bound = false;
	}

	public void unbind(String serviceName) throws JAtmiBrokerException {
		AtmiBroker_ServiceFactoryImpl atmiBroker_ServiceFactoryImpl = serviceFactoryList.get(serviceName);
		if (atmiBroker_ServiceFactoryImpl != null) {
			atmiBroker_ServiceFactoryImpl.unbind();
		}
	}

	public boolean deregister_client(ClientInfo client_info) {
		log.error("NO-OP deregister_client");
		// TODO Auto-generated method stub
		return false;
	}

	public ServiceInfo[] get_all_service_info() {
		log.error("NO-OP get_all_service_info");
		// TODO Auto-generated method stub
		return null;
	}

	public String get_client_callback(ClientInfo client_info) {
		log.error("NO-OP get_client_callback");
		// TODO Auto-generated method stub
		return null;
	}

	public ClientInfo[] get_client_info() {
		log.error("NO-OP get_client_info");
		// TODO Auto-generated method stub
		return null;
	}

	public EnvVariableInfo[] get_environment_variable_info() {
		log.error("NO-OP get_environment_variable_info");
		// TODO Auto-generated method stub
		return null;
	}

	public int get_queue_log(String queue_name) {
		log.error("NO-OP get_queue_log");
		// TODO Auto-generated method stub
		return 0;
	}

	public ServerInfo get_server_info() {
		log.error("NO-OP get_server_info");
		// TODO Auto-generated method stub
		return null;
	}

	public ServiceInfo get_service_info(String service_name) {
		log.error("NO-OP get_service_info");
		// TODO Auto-generated method stub
		return null;
	}

	public int register_client(ClientInfo client_info) {
		log.error("NO-OP register_client");
		// TODO Auto-generated method stub
		return 0;
	}

	public void server_done() {
		log.error("NO-OP server_done");
		// TODO Auto-generated method stub

	}

	public short server_init() {
		log.error("NO-OP server_init");
		// TODO Auto-generated method stub
		return 0;
	}

	public void set_environment_descriptor(String xml_descriptor) {
		log.error("NO-OP set_environment_descriptor");
		// TODO Auto-generated method stub

	}

	public void set_server_descriptor(String xml_descriptor) {
		log.error("NO-OP set_server_descriptor");
		// TODO Auto-generated method stub

	}

	public void set_service_descriptor(String serviceName, String xml_descriptor) {
		log.error("NO-OP set_service_descriptor");
		// TODO Auto-generated method stub

	}

	public void start_service(String service_name) {
		log.error("NO-OP start_service");
		// TODO Auto-generated method stub

	}

	public void stop_service(String service_name) {
		log.error("NO-OP stop_service");
		// TODO Auto-generated method stub

	}
}
