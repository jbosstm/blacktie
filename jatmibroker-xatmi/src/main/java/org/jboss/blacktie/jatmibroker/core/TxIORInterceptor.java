/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat, Inc., and others contributors as indicated
 * by the @authors tag. All rights reserved.
 * See the copyright.txt in the distribution for a
 * full listing of individual contributors.
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License, v. 2.1.
 * This program is distributed in the hope that it will be useful, but WITHOUT A
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
package org.jboss.blacktie.jatmibroker.core;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

import org.omg.CORBA.Any;
import org.omg.CORBA.LocalObject;
import org.omg.CORBA.ORB;
import org.omg.IOP.TaggedComponent;
import org.omg.IOP.Codec;
import org.omg.IOP.CodecPackage.InvalidTypeForEncoding;
import org.omg.PortableInterceptor.IORInfo;
import org.omg.PortableInterceptor.IORInterceptor;

public class TxIORInterceptor extends org.omg.CORBA.LocalObject implements IORInterceptor
{
	private static final Logger log = LogManager.getLogger(TxIORInterceptor.class);

	private Codec codec;
	public static final int TAG_OTS_POLICY = 9056; //31;
	public static final int TAG_INV_POLICY = 32;
	public static final short EITHER = 0;
	public static final short REQUIRES = 1;
	public static final short FORBIDS = 2;
	public static final short ADAPTS = 3;

	public TxIORInterceptor(Codec codec)
	{
		this.codec = codec;
	}

	public String name()
	{
		return TxIORInterceptor.class.getName();
	}

	public void destroy()
	{
	}

	public void establish_components(IORInfo ior_info)
	{
		log.trace("TxIORInterceptor.establish_components");
		try {
			Any any = ORB.init().create_any();
			any.insert_short(EITHER);
			byte[] taggedComponentData = codec.encode_value(any);
			ior_info.add_ior_component(new TaggedComponent(TAG_INV_POLICY, taggedComponentData));
			any = ORB.init().create_any();
			any.insert_short(ADAPTS);
			taggedComponentData = codec.encode_value(any);
			ior_info.add_ior_component(new TaggedComponent(TAG_OTS_POLICY, taggedComponentData));
		} catch (InvalidTypeForEncoding e) {
			log.error("Exception during OTS policy encoding", e);
			throw new RuntimeException("Exception during OTS policy encoding", e);
		}
	}
}
