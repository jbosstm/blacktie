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
package org.jboss.blacktie.jatmibroker.core.administration;

public interface BTServerAdministration {

	public short server_init();

	public void server_done();

	public AtmiBroker.ServerInfo get_server_info();

	public AtmiBroker.ServiceInfo[] get_all_service_info();

	public AtmiBroker.EnvVariableInfo[] get_environment_variable_info();

	public void set_server_descriptor(String xml_descriptor);

	public void set_service_descriptor(String service_name, String xml_descriptor);

	public void set_environment_descriptor(String xml_descriptor);

	public void stop_service(String service_name);

	public void start_service(String service_name);
}
