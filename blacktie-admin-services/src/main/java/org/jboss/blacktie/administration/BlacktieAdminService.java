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

package org.jboss.blacktie.administration;

import java.util.List;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.administration.core.AdministrationProxy;
import org.w3c.dom.Element;

/**
 * This is the JMX interface into the blacktie administration proxy.
 */
public class BlacktieAdminService implements BlacktieAdminServiceMBean {
	private static final Logger log = LogManager
			.getLogger(BlacktieAdminService.class);
	private QueueReaper reaper;
	private AdministrationProxy administrationProxy;

	/**
	 * Start the service
	 */
	public void start() throws Exception {
		administrationProxy = new AdministrationProxy();
		reaper = new QueueReaper(administrationProxy.getBeanServerConnection());
		reaper.startThread();

		log.info("Admin Server Started");
	}

	/**
	 * Stop the service
	 */
	public void stop() throws Exception {
		reaper.stopThread();
		administrationProxy.close();
		log.info("Admin Server Stopped");
	}

	/**
	 * Retrieve the domain name
	 */
	public String getDomainName() {
		return administrationProxy.getDomainName();
	}

	/**
	 * Get the version of the blacktie software
	 */
	public String getSoftwareVersion() {
		return administrationProxy.getSoftwareVersion();
	}

	/**
	 * Pause the domain
	 */
	public Boolean pauseDomain() {
		return administrationProxy.pauseDomain();
	}

	/**
	 * Resume the domain
	 */
	public Boolean resumeDomain() {
		return administrationProxy.resumeDomain();
	}

	/**
	 * List the servers
	 */
	public List<String> getServerList() {
		return administrationProxy.getServerList();
	}

	/**
	 * List the running servers
	 */
	public List<String> listRunningServers() {
		return administrationProxy.listRunningServers();
	}

	/**
	 * List the running ids of a specific server
	 * 
	 * @param serverName
	 *            The name of the server
	 */
	public List<Integer> listRunningInstanceIds(String serverName) {
		return administrationProxy.listRunningInstanceIds(serverName);
	}

	/**
	 * Get the servers status for the domain
	 */
	public Element getServersStatus() {
		return administrationProxy.getServersStatus();
	}

	/**
	 * List the service status for a service
	 * 
	 * @param serverName
	 *            The name of the server
	 * @param serviceName
	 *            The name of the service
	 */
	public Element listServiceStatus(String serverName, String serviceName) {
		return administrationProxy.listServiceStatus(serverName, serviceName);
	}

	public Boolean advertise(String serverName, String serviceName) {
		return administrationProxy.advertise(serverName, serviceName);
	}

	public Boolean unadvertise(String serverName, String serviceName) {
		return administrationProxy.unadvertise(serverName, serviceName);
	}

	public void shutdown(String serverName, int id) {
		administrationProxy.shutdown(serverName, id);
	}

	public long getServiceCounterById(String serverName, int id,
			String serviceName) {
		return administrationProxy.getServiceCounterById(serverName, id,
				serviceName);
	}

	public long getServiceCounter(String serverName, String serviceName) {
		return administrationProxy.getServiceCounter(serverName, serviceName);
	}

	public Boolean reloadDomain() {
		return administrationProxy.reloadDomain();
	}

	public Boolean reloadServer(String serverName) {
		return administrationProxy.reloadServer(serverName);
	}

	public Boolean reloadServerById(String serverName, int id) {
		return administrationProxy.reloadServerById(serverName, id);
	}

	public Element listServiceStatusById(String serverName, int id,
			String serviceName) {
		return administrationProxy.listServiceStatusById(serverName, id,
				serviceName);
	}
}
