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
package org.jboss.blacktie.jatmibroker.xatmi.ejb;

import java.lang.reflect.InvocationTargetException;
import java.rmi.RemoteException;

import org.jboss.blacktie.jatmibroker.core.AtmiBroker_CallbackConverter;
import org.jboss.blacktie.jatmibroker.core.AtmiBroker_Response;
import org.jboss.blacktie.jatmibroker.xatmi.connector.Response;
import org.jboss.blacktie.jatmibroker.xatmi.connector.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.connector.buffers.Buffer;

public class AtmiBrokerCallbackConverterImpl implements
		AtmiBroker_CallbackConverter {
	public AtmiBroker_Response serviceRequest(Object toInvokeOn,
			String serviceName, byte[] bytes, int length, int flags)
			throws IllegalArgumentException, IllegalAccessException,
			InvocationTargetException, RemoteException {

		// TODO unknown -1
		Buffer buffer = new Buffer("unknown", "unknown", length);
		buffer.setData(bytes);
		TPSVCINFO tpsvcinfo = new TPSVCINFO(serviceName, buffer, flags, -1);
		Response response = ((BlacktieService) toInvokeOn).tpservice(tpsvcinfo);
		AtmiBroker_Response atmiBroker_Response = new AtmiBroker_Response(
				response.getRval(), response.getRcode(), response.getResponse()
						.getData(), response.getResponse().getSize(), response
						.getFlags());
		return atmiBroker_Response;
	}
}
