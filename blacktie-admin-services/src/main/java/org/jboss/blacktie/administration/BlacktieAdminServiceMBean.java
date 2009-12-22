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

/**
 * This is the Admin service MBean
 */
public interface BlacktieAdminServiceMBean {
	/**
	 * Start the service
	 * 
	 * @throws Exception
	 */
	void start() throws Exception;

	/**
	 * Stop the service
	 * 
	 * @throws Exception
	 */
	void stop() throws Exception;

	/**
	 * Retrieve the Domain Name
	 */
	public String getDomainName();

	/**
	 * Retrieve the software version of domain
	 */
	public String getSoftwareVersion();

	/**
	 * Retrieve the full list of servers
	 */
	public java.util.List<String> getServerList();

	/**
	 * Discover running servers
	 */
	public java.util.List<String> listRunningServers();
	
	/**
	 * Get current status of domain
	 */
	public Boolean getDomainStatus();

	/**
	 * This calls pauseServer for each server in the domain
	 */
	public Boolean pauseDomain();

	/**
	 * This calls resumeDomain for each server in the domain
	 */
	public Boolean resumeDomain();
	
	/**
	 * Halt servers, update configuration, restart
	 */
	public Boolean reloadDomain();
	
	/**
	 * reload server
	 */
	public Boolean reloadServer(String serverName);

	/**
	 * Retrieves the counter for a service from all servers
	 */
	public long getServiceCounter(String serverName, String serviceName);
	
	/**
	 * Retrieves the counter for a service from specify server
	 */
	public long getServiceCounterById(String serverName, int id, String serviceName);

	/**
	 * Get the list of Ids of currently running servers
	 */
	public java.util.List<Integer> listRunningInstanceIds(String serverName);

	/**
	 * Describe the status of the servers in the domain
	 */
	public org.w3c.dom.Element getServersStatus();
	
	/** 
	 * Describe the service status of server
	 */
	public org.w3c.dom.Element listServiceStatus(String serverName, String serviceName);
	
	public org.w3c.dom.Element listServiceStatusById(String serverName, int id, String serviceName);

	/**
	 * Advertise service
	 */
	public Boolean advertise(String serverName, String serviceName);

	/**
	 * Unadvertise service
	 */
	public Boolean unadvertise(String serverName, String serviceName);

	/**
	 * Shutdown server
	 */
	public void shutdown(String serverName, int id);
}
