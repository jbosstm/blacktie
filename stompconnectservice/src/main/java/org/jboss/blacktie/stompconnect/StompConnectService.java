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
package org.jboss.blacktie.stompconnect;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.codehaus.stomp.jms.StompConnect;

public class StompConnectService implements StompConnectServiceMBean {
	private static final Logger log = LogManager
			.getLogger(StompConnectService.class);
	protected StompConnect connect = new StompConnect();
	private String uri;
	private String connectionFactoryName;

	public void start() throws Exception {
		log.info("Starting StompConnectMBeanImpl: " + uri);
		connect.setJndiName(connectionFactoryName);
		connect.setUri(uri);
		connect.start();
		log.info("Started StompConnectMBeanImpl: " + uri);
	}

	public void stop() throws Exception {
		log.info("Stopping StompConnectMBeanImpl: " + uri);
		connect.stop();
		log.info("Stopped StompConnectMBeanImpl: " + uri);
	}

	public void setConnectionFactoryName(String connectionFactoryName) {
		this.connectionFactoryName = connectionFactoryName;
	}

	public void setUri(String uri) {
		this.uri = uri;
	}
}
