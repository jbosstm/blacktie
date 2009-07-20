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
package org.jboss.blacktie.jatmibroker.admin;

import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.transport.OrbManagement;
import org.omg.CORBA.Object;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.AlreadyBound;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;

import AtmiBroker.Server;
import AtmiBroker.ServerHelper;

public class AdministrationProxy {
	private static final Logger log = LogManager
			.getLogger(AdministrationProxy.class);
	private Server server;
	private OrbManagement orbManagement;

	public AdministrationProxy(Properties properties, String serverName)
			throws InvalidName, NotFound, CannotProceed,
			org.omg.CosNaming.NamingContextPackage.InvalidName,
			AdapterInactive, AlreadyBound {
		orbManagement = new OrbManagement(properties, false);
		server = null;

		log.debug("about to resolve '" + serverName + "'");
		Object serverObject = orbManagement.getNamingContext().resolve(
				orbManagement.getNamingContextExt().to_name(serverName));
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

	public void set_service_descriptor(String service_name,
			String xml_descriptor) {
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

	public void close() {
		orbManagement.close();
	}

}
