/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
 * by the @authors tag. All rights reserved.
 * See the copyright.txt in the distribution for a
 * full listing of individual contributors.
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General public  License, v. 2.1.
 * This program is distributed in the hope that it will be useful, but WITHOUT A
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General public  License for more details.
 * You should have received a copy of the GNU Lesser General public  License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
package org.jboss.blacktie.jatmibroker.xatmi;

import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

public class X_COMMON extends Buffer {

	/**
	 * The default ID
	 */
	private static final long serialVersionUID = 1L;

	private static List<Class> types = new ArrayList<Class>();

	static {
		Class[] x_commonType = new Class[] { short.class, long.class,
				char.class, short[].class, long[].class, char[].class };
		for (int i = 0; i < x_commonType.length; i++) {
			types.add(x_commonType[i]);
		}
	}

	public X_COMMON(String subtype, Properties properties)
			throws ConnectionException {
		super("X_COMMON", subtype, true, types, properties);
	}

	X_COMMON(String subtype, Properties properties, byte[] data)
			throws ConnectionException {
		super("X_COMMON", subtype, true, types, properties);
		setData(data);
		deserialize();
	}

}
