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
package org.jboss.blacktie.jatmibroker.core;

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.proxy.Administration;

public class ServerTest extends TestCase {
	private static final Logger log = LogManager.getLogger(ServerTest.class);

	public void testTODO() {

	}

	public void xtest() throws Exception {
		String[] args = new String[3];
		args[0] = "fooapp";
		args[1] = "foo";
		args[2] = "BAR";
		log.debug(" domain is " + args[0]);
		log.debug(" server is " + args[1]);
		log.debug(" method is " + args[2]);

		Administration serverAdministration = AtmiBrokerServerProxy
				.getAdministration(args, args[0], args[1]);

		if (args[2].equals("server_init")) {
			short aStatus = serverAdministration.server_init();
			log.debug("status is " + aStatus);
		} else if (args[2].equals("server_done")) {
			serverAdministration.server_done();
		} else if (args[2].equals("get_server_info")) {
			AtmiBroker.ServerInfo aServerInfo = serverAdministration
					.get_server_info();
			log.debug("aServerInfo maxChannels " + aServerInfo.maxChannels);
			log.debug("aServerInfo maxSuppliers " + aServerInfo.maxSuppliers);
			log.debug("aServerInfo maxConsumers " + aServerInfo.maxConsumers);
			log.debug("aServerInfo maxReplicas " + aServerInfo.maxReplicas);
			log.debug("aServerInfo logLevel " + aServerInfo.logLevel);
			log.debug("aServerInfo securityType " + aServerInfo.securityType);
			log.debug("aServerInfo orbType " + aServerInfo.orbType);
			log.debug("aServerInfo queueSpaceName "
					+ aServerInfo.queueSpaceName);
			for (int i = 0; i < aServerInfo.serviceNames.length; i++)
				log.debug("aServerInfo serviceNames[" + i + "]"
						+ aServerInfo.serviceNames[i]);
		} else if (args[2].equals("get_all_service_info")) {
			AtmiBroker.ServiceInfo[] aServiceInfo = serverAdministration
					.get_all_service_info();
			for (int i = 0; i < aServiceInfo.length; i++) {
				log.debug("aServiceInfo[" + i + "] " + aServiceInfo[i]);
				log.debug("aServiceInfo[" + i + "] serviceName "
						+ aServiceInfo[i].serviceName);
				log.debug("aServiceInfo[" + i + "] poolSize "
						+ aServiceInfo[i].poolSize);
				log.debug("aServiceInfo[" + i + "] securityType "
						+ aServiceInfo[i].securityType);
			}
		} else if (args[2].equals("get_environment_variable_info")) {
			AtmiBroker.EnvVariableInfo[] aEnvVarInfo = serverAdministration
					.get_environment_variable_info();
			for (int i = 0; i < aEnvVarInfo.length; i++)
				log.debug("aEnvVarInfo[" + i + "] name " + aEnvVarInfo[i].name
						+ " value " + aEnvVarInfo[i].value);
		} else if (args[2].equals("set_server_descriptor")) {
			String xml_descriptor = "<SERVER_INFO></SERVER_INFO>";
			serverAdministration.set_server_descriptor(xml_descriptor);
		} else if (args[2].equals("set_service_descriptor")) {
			String aServiceName = "Bar";
			String xml_descriptor = "<SERVICE_INFO></SERVICE_INFO>";
			serverAdministration.set_service_descriptor(aServiceName,
					xml_descriptor);
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

		serverAdministration.close();
	}
}
