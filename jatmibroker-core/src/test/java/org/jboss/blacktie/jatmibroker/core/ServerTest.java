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

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.administration.BTServerAdministration;

public class ServerTest extends TestCase {
	private static final Logger log = LogManager.getLogger(ServerTest.class);

	public void testTODO() {

	}

	public void xtest() throws Exception {
		String[] args = new String[3];
		args[0] = "default";
		args[1] = "foo";
		args[2] = "BAR";
		log.debug(" company is " + args[0]);
		log.debug(" server is " + args[1]);
		log.debug(" method is " + args[2]);

		BTServerAdministration serverAdministration = AtmiBrokerServerImpl.getAdministration(args, args[0], args[1]);

		if (args[2].equals("get_queue_log")) {
			int queueLogId = serverAdministration.get_queue_log("FooQueue");
			log.debug("FooServer queueLogId is " + queueLogId);
		} else if (args[2].equals("server_init")) {
			short aStatus = serverAdministration.server_init();
			log.debug("status is " + aStatus);
		} else if (args[2].equals("get_client_callback")) {
			AtmiBroker.ClientInfo client_info = new AtmiBroker.ClientInfo();
			String aCallbackIOR = serverAdministration.get_client_callback(client_info);
			log.debug("aCallbackIOR is " + aCallbackIOR);
		} else if (args[2].equals("server_done")) {
			serverAdministration.server_done();
		} else if (args[2].equals("get_server_info")) {
			AtmiBroker.ServerInfo aServerInfo = serverAdministration.get_server_info();
			log.debug("aServerInfo maxChannels " + aServerInfo.maxChannels);
			log.debug("aServerInfo maxSuppliers " + aServerInfo.maxSuppliers);
			log.debug("aServerInfo maxConsumers " + aServerInfo.maxConsumers);
			log.debug("aServerInfo maxReplicas " + aServerInfo.maxReplicas);
			log.debug("aServerInfo logLevel " + aServerInfo.logLevel);
			log.debug("aServerInfo securityType " + aServerInfo.securityType);
			log.debug("aServerInfo orbType " + aServerInfo.orbType);
			log.debug("aServerInfo queueSpaceName " + aServerInfo.queueSpaceName);
			for (int i = 0; i < aServerInfo.serviceNames.length; i++)
				log.debug("aServerInfo serviceNames[" + i + "]" + aServerInfo.serviceNames[i]);
		} else if (args[2].equals("get_all_service_info")) {
			AtmiBroker.ServiceInfo[] aServiceInfo = serverAdministration.get_all_service_info();
			for (int i = 0; i < aServiceInfo.length; i++) {
				log.debug("aServiceInfo[" + i + "] " + aServiceInfo[i]);
				log.debug("aServiceInfo[" + i + "] serviceName " + aServiceInfo[i].serviceName);
				log.debug("aServiceInfo[" + i + "] maxSize " + aServiceInfo[i].maxSize);
				log.debug("aServiceInfo[" + i + "] minSize " + aServiceInfo[i].minSize);
				log.debug("aServiceInfo[" + i + "] minAvailableSize " + aServiceInfo[i].minAvailableSize);
				log.debug("aServiceInfo[" + i + "] inUse " + aServiceInfo[i].inUse);
				log.debug("aServiceInfo[" + i + "] available " + aServiceInfo[i].available);
				log.debug("aServiceInfo[" + i + "] logLevel " + aServiceInfo[i].logLevel);
				log.debug("aServiceInfo[" + i + "] securityType " + aServiceInfo[i].securityType);
			}
		} else if (args[2].equals("get_service_info")) {
			AtmiBroker.ServiceInfo aServiceInfo = serverAdministration.get_service_info(args[3]);
			log.debug("aServiceInfo " + aServiceInfo);
			log.debug("aServiceInfo serviceName " + aServiceInfo.serviceName);
			log.debug("aServiceInfo maxSize " + aServiceInfo.maxSize);
			log.debug("aServiceInfo minSize " + aServiceInfo.minSize);
			log.debug("aServiceInfo minAvailableSize " + aServiceInfo.minAvailableSize);
			log.debug("aServiceInfo inUse " + aServiceInfo.inUse);
			log.debug("aServiceInfo available " + aServiceInfo.available);
			log.debug("aServiceInfo logLevel " + aServiceInfo.logLevel);
			log.debug("aServiceInfo securityType " + aServiceInfo.securityType);
		} else if (args[2].equals("get_environment_variable_info")) {
			AtmiBroker.EnvVariableInfo[] aEnvVarInfo = serverAdministration.get_environment_variable_info();
			for (int i = 0; i < aEnvVarInfo.length; i++)
				log.debug("aEnvVarInfo[" + i + "] name " + aEnvVarInfo[i].name + " value " + aEnvVarInfo[i].value);
		} else if (args[2].equals("get_client_info")) {
			AtmiBroker.ClientInfo[] aClientInfo = serverAdministration.get_client_info();
			for (int i = 0; i < aClientInfo.length; i++) {
				log.debug("aClientInfo[" + i + "] " + aClientInfo[i]);
				log.debug("aClientInfo[" + i + "] maxChannels " + aClientInfo[i].maxChannels);
				log.debug("aClientInfo[" + i + "] maxSuppliers " + aClientInfo[i].maxSuppliers);
				log.debug("aClientInfo[" + i + "] maxConsumers " + aClientInfo[i].maxConsumers);
				log.debug("aClientInfo[" + i + "] maxReplicas " + aClientInfo[i].maxReplicas);
				log.debug("aClientInfo[" + i + "] logLevel " + aClientInfo[i].logLevel);
			}
		} else if (args[2].equals("set_server_descriptor")) {
			String xml_descriptor = "<SERVER_INFO></SERVER_INFO>";
			serverAdministration.set_server_descriptor(xml_descriptor);
		} else if (args[2].equals("set_service_descriptor")) {
			String aServiceName = "Bar";
			String xml_descriptor = "<SERVICE_INFO></SERVICE_INFO>";
			serverAdministration.set_service_descriptor(aServiceName, xml_descriptor);
		} else if (args[2].equals("set_environment_descriptor")) {
			String xml_descriptor = "<ENVIRONMENT_INFO></ENVIRONMENT_INFO>";
			serverAdministration.set_environment_descriptor(xml_descriptor);
		} else if (args[2].equals("stop_service")) {
			String aServiceName = "BAR";
			serverAdministration.stop_service(aServiceName);
		} else if (args[2].equals("start_service")) {
			String aServiceName = "BAR";
			serverAdministration.start_service(aServiceName);
		}

		AtmiBrokerServerImpl.discardOrb();
	}
}
