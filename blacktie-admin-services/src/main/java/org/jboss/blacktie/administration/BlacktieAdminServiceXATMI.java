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

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;

import javax.ejb.ActivationConfigProperty;
import javax.ejb.MessageDriven;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.administration.core.AdministrationProxy;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;
import org.jboss.blacktie.jatmibroker.xatmi.mdb.MDBBlacktieService;
import org.jboss.ejb3.annotation.Depends;
import org.w3c.dom.Element;

@MessageDriven(activationConfig = {
		@ActivationConfigProperty(propertyName = "destinationType", propertyValue = "javax.jms.Queue"),
		@ActivationConfigProperty(propertyName = "destination", propertyValue = "queue/BTDomainAdmin") })
@Depends("jboss.messaging.destination:service=Queue,name=BTDomainAdmin")
@javax.ejb.TransactionAttribute(javax.ejb.TransactionAttributeType.NOT_SUPPORTED)
public class BlacktieAdminServiceXATMI extends MDBBlacktieService implements
		javax.jms.MessageListener {
	private static final Logger log = LogManager
			.getLogger(BlacktieAdminServiceXATMI.class);

	private AdministrationProxy administrationProxy;

	public BlacktieAdminServiceXATMI() throws IOException,
			ConfigurationException, ConnectionException {
		super("BTDomainAdmin");
		administrationProxy = new AdministrationProxy();
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		log.debug("Message received");
		X_OCTET recv = (X_OCTET) svcinfo.getBuffer();
		String string = new String(recv.getByteArray());
		StringTokenizer parameters = new StringTokenizer(string, ",", false);
		String operation = parameters.nextToken();
		byte[] toReturn = null;
		try {
			if (operation.equals("getDomainName")) {
				String response = getDomainName();
				toReturn = response.getBytes();
			} else if (operation.equals("getSoftwareVersion")) {
				String response = getSoftwareVersion();
				toReturn = response.getBytes();
			} else if (operation.equals("pauseDomain")) {
				boolean response = pauseDomain();
				toReturn = convertBoolean(response);
			} else if (operation.equals("resumeDomain")) {
				boolean response = resumeDomain();
				toReturn = convertBoolean(response);
			} else if (operation.equals("getServerList")) {
				List<String> response = getServerList();
				toReturn = convertListString(response);
			} else if (operation.equals("listRunningServers")) {
				List<String> response = listRunningServers();
				toReturn = convertListString(response);
			} else if (operation.equals("listRunningInstanceIds")) {
				String serverName = getString(parameters);
				List<Integer> response = listRunningInstanceIds(serverName);
				toReturn = convertListInt(response);
			} else if (operation.equals("getServersStatus")) {
				Element response = getServersStatus();
				toReturn = response.toString().getBytes();
			} else if (operation.equals("listServiceStatus")) {
				String serverName = getString(parameters);
				String serviceName = getString(parameters);
				Element response = listServiceStatus(serverName, serviceName);
				toReturn = response.toString().getBytes();
			} else if (operation.equals("advertise")) {
				String serverName = getString(parameters);
				String serviceName = getString(parameters);
				boolean response = advertise(serverName, serviceName);
				toReturn = convertBoolean(response);
			} else if (operation.equals("unadvertise")) {
				String serverName = getString(parameters);
				String serviceName = getString(parameters);
				boolean response = unadvertise(serverName, serviceName);
				toReturn = convertBoolean(response);
			} else if (operation.equals("shutdown")) {
				String serverName = getString(parameters);
				int id = getInt(parameters);
				shutdown(serverName, id);
				toReturn = new byte[1];
				toReturn[0] = 1;
			} else if (operation.equals("getServiceCounterById")) {
				String serverName = getString(parameters);
				int id = getInt(parameters);
				String serviceName = getString(parameters);
				long response = getServiceCounterById(serverName, id,
						serviceName);
				toReturn = convertLong(response);
			} else if (operation.equals("getServiceCounter")) {
				String serverName = getString(parameters);
				String serviceName = getString(parameters);
				long response = getServiceCounter(serverName, serviceName);
				toReturn = convertLong(response);
			} else if (operation.equals("reloadDomain")) {
				boolean response = reloadDomain();
				toReturn = convertBoolean(response);
			} else if (operation.equals("reloadServer")) {
				String serverName = getString(parameters);
				boolean response = reloadServer(serverName);
				toReturn = convertBoolean(response);
			} else if (operation.equals("listServiceStatusById")) {
				String serverName = getString(parameters);
				int id = getInt(parameters);
				String serviceName = getString(parameters);
				Element response = listServiceStatusById(serverName, id,
						serviceName);
				toReturn = response.toString().getBytes();
			}

			X_OCTET buffer = (X_OCTET) svcinfo.tpalloc("X_OCTET", null);
			buffer.setByteArray(toReturn);
			log.debug("Responding");
			return new Response(Connection.TPSUCCESS, 0, buffer,
					toReturn.length, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		} catch (IOException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}

	private byte[] convertListInt(List<Integer> response) {
		StringBuffer toReturn = new StringBuffer();
		Iterator<Integer> iterator = response.iterator();
		while (iterator.hasNext()) {
			toReturn.append(iterator.next());
			toReturn.append(',');
		}
		return toReturn.toString().getBytes();
	}

	private byte[] convertLong(long response) throws IOException {
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		DataOutputStream dos = new DataOutputStream(baos);
		ByteBuffer bbuf = ByteBuffer.allocate(Buffer.LONG_SIZE);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		bbuf.putLong(response);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		long toWrite = bbuf.getLong(0);
		dos.writeLong(toWrite);
		dos.flush();
		baos.flush();
		return baos.toByteArray();
	}

	private byte[] convertListString(List<String> response) {
		StringBuffer toReturn = new StringBuffer();
		Iterator<String> iterator = response.iterator();
		while (iterator.hasNext()) {
			toReturn.append(iterator.next());
			toReturn.append(',');
		}
		return toReturn.toString().getBytes();
	}

	private byte[] convertBoolean(boolean response) {
		byte[] toReturn = new byte[1];
		if (response) {
			toReturn[0] = 1;
		}
		return toReturn;
	}

	private String getString(StringTokenizer parameters) {
		return parameters.nextToken();
	}

	private int getInt(StringTokenizer parameters) {
		return Integer.parseInt(parameters.nextToken());
	}

	/**
	 * Retrieve the domain name
	 */
	private String getDomainName() {
		return administrationProxy.getDomainName();
	}

	/**
	 * Get the version of the blacktie software
	 */
	private String getSoftwareVersion() {
		return administrationProxy.getSoftwareVersion();
	}

	/**
	 * Pause the domain
	 */
	private Boolean pauseDomain() {
		return administrationProxy.pauseDomain();
	}

	/**
	 * Resume the domain
	 */
	private Boolean resumeDomain() {
		return administrationProxy.resumeDomain();
	}

	/**
	 * List the servers
	 */
	private List<String> getServerList() {
		return administrationProxy.getServerList();
	}

	/**
	 * List the running servers
	 */
	private List<String> listRunningServers() {
		return administrationProxy.listRunningServers();
	}

	/**
	 * List the running ids of a specific server
	 * 
	 * @param serverName
	 *            The name of the server
	 */
	private List<Integer> listRunningInstanceIds(String serverName) {
		return administrationProxy.listRunningInstanceIds(serverName);
	}

	/**
	 * Get the servers status for the domain
	 */
	private Element getServersStatus() {
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
	private Element listServiceStatus(String serverName, String serviceName) {
		return administrationProxy.listServiceStatus(serverName, serviceName);
	}

	/**
	 * Advertise a new service
	 * 
	 * @param serverName
	 *            The name of the server
	 * @param serviceName
	 *            The name of the service
	 */
	private Boolean advertise(String serverName, String serviceName) {
		return administrationProxy.advertise(serverName, serviceName);
	}

	/**
	 * Unadvertise a new service
	 * 
	 * @param serverName
	 *            The name of the server
	 * @param serviceName
	 *            The name of the service
	 */
	private Boolean unadvertise(String serverName, String serviceName) {
		return administrationProxy.unadvertise(serverName, serviceName);
	}

	/**
	 * Shutdown a server
	 * 
	 * @param serverName
	 *            The name of the server
	 * @param id
	 *            The id of the server
	 */
	private void shutdown(String serverName, int id) {
		administrationProxy.shutdown(serverName, id);
	}

	/**
	 * Get the service counter and restrict it to a certain server, 0 for all.
	 * 
	 * @param serverName
	 *            The name of the server
	 * @param id
	 *            The id of the server
	 * @param serviceName
	 *            The name of the service
	 */
	private long getServiceCounterById(String serverName, int id,
			String serviceName) {
		return administrationProxy.getServiceCounterById(serverName, id,
				serviceName);
	}

	/**
	 * Get the service counter for the domain.
	 * 
	 * @param serverName
	 *            The name of the server
	 * @param serviceName
	 *            The name of the service
	 */

	private long getServiceCounter(String serverName, String serviceName) {
		return administrationProxy.getServiceCounter(serverName, serviceName);
	}

	/**
	 * Reload the domain
	 */
	private Boolean reloadDomain() {
		return administrationProxy.reloadDomain();
	}

	/**
	 * Reload the server (causes the server to update its configuration and
	 * restart.
	 * 
	 * @param serverName
	 *            The name of the server
	 */
	private Boolean reloadServer(String serverName) {
		return administrationProxy.reloadServer(serverName);
	}

	/**
	 * List the status of a service giving an optional id, 0 is all servers.
	 * 
	 * @param serverName
	 *            The name of the server
	 * @param id
	 *            The id of the server
	 * @param serviceName
	 *            The name of the service
	 */
	private Element listServiceStatusById(String serverName, int id,
			String serviceName) {
		return administrationProxy.listServiceStatusById(serverName, id,
				serviceName);
	}
}