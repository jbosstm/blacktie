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
package org.jboss.blacktie.jatmibroker.core.proxy;

import org.jboss.blacktie.jatmibroker.core.JAtmiBrokerException;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.CosNotifyChannelAdmin.EventChannelFactory;
import org.omg.CosTransactions.TransactionFactory;

public interface AtmiBrokerServer {
	/**
	 * This method will retrieve any information from the callback that has been
	 * sent.
	 * 
	 * @param odata
	 * @param olen
	 * @param flags
	 * @param event
	 * @return
	 */
	public short dequeue_data(AtmiBroker.octetSeqHolder odata, org.omg.CORBA.IntHolder olen, int flags, org.omg.CORBA.IntHolder event);

	public TransactionFactory getTransactionFactory(String transactionManagerServiceName) throws NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName;

	public EventChannelFactory getEventChannelFactory() throws InvalidName;

	public void close();

	public AtmiBrokerServiceManager getServiceManagerProxy(String serviceName) throws JAtmiBrokerException;
}
