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
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.administration.BTServerAdministration;
import org.jboss.blacktie.jatmibroker.core.proxy.AtmiBrokerServer;
import org.jboss.blacktie.jatmibroker.core.proxy.AtmiBrokerServiceFactory;
import org.omg.CORBA.IntHolder;
import org.omg.CORBA.ORB;
import org.omg.CORBA.Object;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NameComponent;
import org.omg.CosNaming.NamingContext;
import org.omg.CosNaming.NamingContextExt;
import org.omg.CosNaming.NamingContextExtHelper;
import org.omg.CosNaming.NamingContextHelper;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.CosNotifyChannelAdmin.EventChannelFactory;
import org.omg.CosNotifyChannelAdmin.EventChannelFactoryHelper;
import org.omg.CosTransactions.TransactionFactory;
import org.omg.CosTransactions.TransactionFactoryHelper;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAHelper;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;
import org.omg.PortableServer.POAPackage.AdapterNonExistent;
import org.omg.PortableServer.POAPackage.InvalidPolicy;
import org.omg.PortableServer.POAPackage.ServantAlreadyActive;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import AtmiBroker.Server;
import AtmiBroker.ServerHelper;
import AtmiBroker.octetSeqHolder;

public class AtmiBrokerServerImpl implements BTServerAdministration, AtmiBrokerServer, Runnable {

	private static final Logger log = LogManager.getLogger(AtmiBrokerServerImpl.class);
	private static final String CorbaOrbClassProp = "org.omg.CORBA.ORBClass";
	private static final String CorbaOrbClassValue = "org.jacorb.orb.ORB";
	private static final String CorbaSingletonClassProp = "org.omg.CORBA.ORBSingletonClass";
	private static final String CorbaSingletonClassValue = "org.jacorb.orb.ORBSingleton";
	private Server server;
	private org.omg.CORBA.Object serverObject;
	private volatile boolean finished;
	static POA root_poa = null;
	static ORB orb = null;
	static NamingContextExt nce = null;
	static NamingContext nc = null;
	private ClientCallbackImpl clientCallbackImpl;

	private Thread callbackThread;
	private ClientInfo clientInfo;

	private Map<String, AtmiBrokerServiceFactory> proxies = new HashMap<String, AtmiBrokerServiceFactory>();
	private String serverName;
	private String companyName;
	private String[] args;

	public synchronized static AtmiBrokerServer getProxy(Properties properties, String userName, String userPassword) throws JAtmiBrokerException {
		AtmiBrokerServerImpl instance = null;
		try {
			instance = new AtmiBrokerServerImpl(properties);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not connect to server", t);
		}
		try {
			instance.createClientCallback(userName, userPassword);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not create callback", t);
		}
		return instance;
	}

	public synchronized static BTServerAdministration getAdministration(String[] args, String namingContextExt, String serverName) throws JAtmiBrokerException {
		try {
			return new AtmiBrokerServerImpl(args, namingContextExt, serverName);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not load administration", t);
		}
	}

	public static void discardOrb() {
		AtmiBrokerServerImpl.orb.shutdown(true);
		AtmiBrokerServerImpl.orb.destroy();
	}

	protected AtmiBrokerServerImpl(Properties properties) throws InvalidName, NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName, AdapterInactive {
		this.companyName = properties.getProperty("blacktie.company.name");
		this.serverName = properties.getProperty("blacktie.server.name");
		String property = properties.getProperty("blacktie.orb.args");
		int numberOfOrbArgs = Integer.parseInt(property);
		List<String> orbArgs = new ArrayList<String>(numberOfOrbArgs);
		for (int i = 1; i <= numberOfOrbArgs; i++) {
			orbArgs.add(properties.getProperty("blacktie.orb.arg." + i));
		}
		this.args = orbArgs.toArray(new String[] {});
		connectToORBAndServer(args, companyName, serverName);
	}

	protected AtmiBrokerServerImpl(String[] args, String namingContextExt, String serverName) throws InvalidName, NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName, AdapterInactive {
		this.serverName = serverName;
		connectToORBAndServer(args, namingContextExt, serverName);
	}

	private void createClientCallback(String username, String password) throws AdapterNonExistent, InvalidPolicy, ServantAlreadyActive, WrongPolicy, ServantNotActive {
		log.debug("createClientCallback create client callback ");
		clientInfo = new ClientInfo();
		clientInfo.user_id = username;
		clientInfo.user_password = password;
		clientCallbackImpl = new ClientCallbackImpl(root_poa, serverName);
		clientInfo.callback_ior = clientCallbackImpl.getCallbackIOR();
		callbackThread = new Thread(this);
		callbackThread.setDaemon(true);
		callbackThread.start();
	}

	public static void ConnectToORB(String[] args, String namingContextExt) throws InvalidName, NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName, AdapterInactive {
		log.debug("ServerProxy's connectToORB args: " + args + " namingContext: " + namingContextExt);

		if (orb == null) {
			java.util.Properties p = new java.util.Properties();
			log.debug("setting properities");
			p.setProperty(CorbaOrbClassProp, CorbaOrbClassValue);
			p.setProperty(CorbaSingletonClassProp, CorbaSingletonClassValue);
			log.debug("set properities");
			log.debug(" initing orb");
			orb = org.omg.CORBA.ORB.init(args, p);
			log.debug(" inited orb");
		}

		if (nce == null) {
			log.debug(" resolving NameService");
			nce = NamingContextExtHelper.narrow(orb.resolve_initial_references("NameService"));
			log.debug("NamingContextExt is " + nce);
		}

		if (nc == null) {
			/**/try {
				log.debug(" creating NamingContext");
				NameComponent[] aNameComponentArray = new NameComponent[1];
				aNameComponentArray[0] = new NameComponent(namingContextExt, "");
				nc = nce.bind_new_context(aNameComponentArray);
				log.debug(" created NamingContext");
			} catch (Exception e) // */
			{
				log.debug(" resolving NamingContext");
				org.omg.CORBA.Object aObject = nce.resolve_str(namingContextExt);
				log.debug("NamingContext Object is " + aObject);

				nc = NamingContextHelper.narrow(aObject);
			}
			log.debug("NamingContext is " + nc);
		}
		if (root_poa == null) {
			// inititialize POA
			log.debug("resolving roo_poa");
			root_poa = POAHelper.narrow(orb.resolve_initial_references("RootPOA"));
			log.debug("roo_poa is " + root_poa);
			root_poa.the_POAManager().activate();
			log.debug("roo_poa is activated ");
		}
		log.debug(" finished & returning from ConnectToORBWithNameServiceProp");
	}

	public void connectToORBAndServer(String[] args, String namingContextExt, String serverName) throws InvalidName, NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName, AdapterInactive {
		server = null;
		log.debug("ServerProxy's connectToORBAndServer args: " + args + " namingContext: " + namingContextExt + " ServerName: " + serverName);

		ConnectToORB(args, namingContextExt);

		log.debug("about to resolve '" + serverName + "'");

		serverObject = nc.resolve(nce.to_name(serverName));
		log.debug("Server Object is " + serverObject);
		log.debug("Server class is " + serverObject.getClass().getName());

		server = ServerHelper.narrow(serverObject);
		log.debug("Server is " + server);
	}

	public short server_init() {
		return server.server_init();
	}

	public void server_done() {
		server.server_done();
	}

	public AtmiBroker.ServerInfo get_server_info() {
		return server.get_server_info();
	}

	public AtmiBroker.ServiceInfo[] get_all_service_info() {
		return server.get_all_service_info();
	}

	public AtmiBroker.EnvVariableInfo[] get_environment_variable_info() {
		return server.get_environment_variable_info();
	}

	public void set_server_descriptor(String xml_descriptor) {
		server.set_server_descriptor(xml_descriptor);
	}

	public void set_service_descriptor(String service_name, String xml_descriptor) {
		server.set_service_descriptor(service_name, xml_descriptor);
	}

	public void set_environment_descriptor(String xml_descriptor) {
		server.set_environment_descriptor(xml_descriptor);
	}

	public void stop_service(String service_name) {
		server.stop_service(service_name);
	}

	public void start_service(String service_name) {
		server.start_service(service_name);
	}

	public Object resolve(String name) throws NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName {
		return AtmiBrokerServerImpl.nc.resolve(AtmiBrokerServerImpl.nce.to_name(name));
	}

	public void run() {
		AtmiBrokerServerImpl.orb.run();
	}

	public synchronized void stopThread() {
		finished = true;
		this.notify();
	}

	public EventChannelFactory getEventChannelFactory() throws InvalidName {
		org.omg.CORBA.Object obj = orb.resolve_initial_references("NotificationService");
		return EventChannelFactoryHelper.narrow(obj);
	}

	public TransactionFactory getTransactionFactory(String transactionManagerServiceName) throws NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName {
		org.omg.CORBA.Object aObject = nce.resolve_str(transactionManagerServiceName);
		return TransactionFactoryHelper.narrow(aObject);
	}

	public void close() {
		Iterator<AtmiBrokerServiceFactory> iterator = proxies.values().iterator();
		while (iterator.hasNext()) {
			iterator.next().close();
		}
	}

	String getClientCallbackIOR() {
		return clientInfo.callback_ior;
	}

	public AtmiBrokerServiceFactory getServiceFactoryProxy(String serviceName) throws JAtmiBrokerException {
		AtmiBrokerServiceFactory proxy = proxies.get(serviceName);
		if (proxy == null) {
			try {
				proxy = AtmiBrokerServiceFactoryImpl.getProxy(this, serviceName);
				proxies.put(serviceName, proxy);
			} catch (Throwable t) {
				throw new JAtmiBrokerException("Could not load service manager proxy for: " + serviceName, t);
			}
		}
		return proxy;
	}

	public short dequeue_data(octetSeqHolder odata, IntHolder olen, int flags, IntHolder event) {
		return clientCallbackImpl.dequeue_data(odata, olen, flags, event);
	}
}
