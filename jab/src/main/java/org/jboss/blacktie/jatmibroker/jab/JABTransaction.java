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
package org.jboss.blacktie.jatmibroker.jab;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.CosTransactions.Control;
import org.omg.CosTransactions.Terminator;
import org.omg.CosTransactions.TransactionFactory;
import org.omg.CosTransactions.Unavailable;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;

public class JABTransaction {
	private static final Logger log = LogManager.getLogger(JABTransaction.class);
	static TransactionFactory transactionFactory;
	private JABSession jabSession;
	private int timeout;
	private Control control;
	private Terminator terminator;

	public JABTransaction(JABSession aJABSession, int aTimeout) throws JABException, NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName, InvalidName, AdapterInactive {
		log.debug("JABTransaction constructor ");

		jabSession = aJABSession;
		timeout = aTimeout;

		control = null;
		terminator = null;

		transactionFactory = jabSession.getServerProxy().getTransactionFactory(jabSession.getJABSessionAttributes().getTransactionManagerName());
		log.debug(" creating Control");
		control = transactionFactory.create(timeout);
		log.debug(" created Control " + control);

		try {
			terminator = control.get_terminator();
			log.debug("Terminator is " + terminator);
		} catch (Unavailable e) {
			throw new JABException(e);
		}
	}

	Control getControl() {
		log.debug("JABTransaction getControl ");
		return control;
	}

	public JABSession getSession() {
		log.debug("JABTransaction getSession ");
		return jabSession;
	}

	public void commit() throws JABException {
		log.debug("JABTransaction commit ");

		try {
			log.debug("calling commit");
			terminator.commit(true);
			log.debug("called commit on terminator ");
		} catch (Exception e) {
			throw new JABException(e);
		}
	}

	public void rollback() throws JABException {
		log.debug("JABTransaction rollback ");

		try {
			log.debug("calling rollback");
			terminator.rollback();
			log.debug("called rollback on terminator ");
		} catch (Exception e) {
			throw new JABException(e);
		}
	}

}
