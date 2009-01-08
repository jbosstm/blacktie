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
package org.jboss.blacktie.jatmibroker.ejb.connector;

import org.jboss.blacktie.jatmibroker.ejb.connector.buffers.Buffer;

/**
 * This is the connector to remote Blacktie services.
 */
public interface Connector {
	// AVAILABLE FLAGS
	int TPNOBLOCK = 0x00000001;
	int TPSIGRSTRT = 0x00000002;
	int TPNOREPLY = 0x00000004;
	int TPNOTRAN = 0x00000008;
	int TPTRAN = 0x00000010;
	int TPNOTIME = 0x00000020;
	int TPGETANY = 0x00000080;
	int TPNOCHANGE = 0x00000100;
	int TPCONV = 0x00000400;
	int TPSENDONLY = 0x00000800;
	int TPRECVONLY = 0x00001000;

	// ERROR CONDITIONS
	public int TPEBADDESC = 2;
	public int TPEBLOCK = 3;
	public int TPEINVAL = 4;
	public int TPELIMIT = 5;
	public int TPENOENT = 6;
	public int TPEOS = 7;
	public int TPEPROTO = 9;
	public int TPESVCERR = 10;
	public int TPESVCFAIL = 11;
	public int TPESYSTEM = 12;
	public int TPETIME = 13;
	public int TPETRAN = 14;
	public int TPGOTSIG = 15;
	public int TPEITYPE = 17;
	public int TPEOTYPE = 18;
	public int TPEEVENT = 22;
	public int TPEMATCH = 23;

	/**
	 * Synchronous call
	 * 
	 * @param svc
	 *            The name of the service to call
	 * @param idata
	 *            The inbound data
	 * @param flags
	 *            The flags to use
	 * @return The returned buffer
	 */
	Response tpcall(String svc, Buffer idata, int flags) throws ConnectorException;

	/**
	 * Asynchronous call
	 * 
	 * @param svc
	 *            The name of the service to call
	 * @param idata
	 *            The inbound data
	 * @param flags
	 *            The flags to use
	 * @return The connection descriptor
	 */
	// int tpacall(String svc, TypedBuffer idata, int flags) throws
	// ConnectorException;
	/**
	 * Get the reply from the server
	 * 
	 * @param cd
	 *            The connection descriptor to use
	 * @param flags
	 *            The flags to use
	 * @return The response from the server
	 */
	// Response tpgetrply(int cd, int flags) throws ConnectorException;
	/**
	 * Cancel the outstanding asynchronous call.
	 * 
	 * @param cd
	 *            The connection descriptor
	 * @param flags
	 *            The flags to use
	 */
	// void tpcancel(int cd, int flags) throws ConnectorException;
	/**
	 * Handle the initiation of a conversation with the server
	 * 
	 * @param svc
	 *            The name of the service
	 * @param idata
	 *            The outbound buffer
	 * @param flags
	 *            The flags to use
	 * @return The connection descriptor
	 */
	// int tpconnect(String svc, TypedBuffer idata, int flags) throws
	// ConnectorException;
	/**
	 * Send a buffer to a remote server in a conversation
	 * 
	 * @param cd
	 *            The connection descriptor
	 * @param idata
	 *            The outbound data
	 * @param flags
	 *            The flags to use
	 */
	// void tpsend(int cd, TypedBuffer idata, int flags) throws
	// ConnectorException;
	/**
	 * Received the next response in a conversation
	 * 
	 * @param cd
	 *            The connection descriptor to use
	 * @param flags
	 *            The flags to use
	 * @return The next response
	 */
	// Response tprecv(int cd, int flags) throws ConnectorException;
	/**
	 * Close the conversation with the remote service.
	 * 
	 * @param cd
	 *            The connection descriptor to use
	 */
	// void tpdiscon(int cd) throws ConnectorException;
	/**
	 * Advertise a service dynamically at runtime
	 * 
	 * @param svc
	 *            The name of the service
	 * @param tocall
	 *            The service to call
	 * @throws ConnectorException
	 *             If the method could not be advertised
	 */
	void tpadvertise(String svc, Class tocall) throws ConnectorException;

	/**
	 * Unadvertise the service
	 * 
	 * @param svc
	 *            The name of the service to unadvertise
	 * @throws ConnectorException
	 *             If the method could not be deregistered
	 */
	void tpunadvertise(String svc) throws ConnectorException;
}
