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

import java.util.Hashtable;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.transport.OrbManagement;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.CosTransactions.Control;
import org.omg.CosTransactions.Terminator;
import org.omg.CosTransactions.TransactionFactory;
import org.omg.CosTransactions.TransactionFactoryHelper;
import org.omg.CosTransactions.Unavailable;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;

public class JABTransaction {
	private static final Logger log = LogManager
			.getLogger(JABTransaction.class);
	static TransactionFactory transactionFactory;
	private JABSession jabSession;
	private int timeout;
	private Control control;
	private Terminator terminator;

	private Hashtable _childThreads;
	private boolean active = true;
	private OrbManagement orbManagement;

	public void finalize() throws Throwable {
		// TODO use ThreadActionData.purgeAction(this); not popAction
		ThreadActionData.popAction();
		super.finalize();
	}

	public JABTransaction(JABSession aJABSession, int aTimeout)
			throws JABException, NotFound, CannotProceed,
			org.omg.CosNaming.NamingContextPackage.InvalidName, InvalidName,
			AdapterInactive {
		log.debug("JABTransaction constructor");

		jabSession = aJABSession;
		timeout = aTimeout;

		control = null;
		terminator = null;

		Properties properties = jabSession.getJABSessionAttributes()
				.getProperties();
		orbManagement = new OrbManagement(properties, false);
		String toLookup = (String) properties.get("blacktie.trans.factoryid");
		org.omg.CORBA.Object aObject = orbManagement.getNamingContextExt()
				.resolve_str(toLookup);
		transactionFactory = TransactionFactoryHelper.narrow(aObject);

		log.debug(" creating Control");
		control = transactionFactory.create(timeout);
		ThreadActionData.pushAction(this);
		log.debug(" created Control " + control);

		try {
			terminator = control.get_terminator();
			log.debug("Terminator is " + terminator);
		} catch (Unavailable e) {
			throw new JABException(e);
		}
	}

	public Control getControl() {
		log.debug("JABTransaction getControl");
		return control;
	}

	public JABSession getSession() {
		log.debug("JABTransaction getSession");
		return jabSession;
	}

	public void commit() throws JABException {
		log.debug("JABTransaction commit");

		try {
			log.debug("calling commit");
			terminator.commit(true);
			active = false;
			ThreadActionData.popAction();
			log.debug("called commit on terminator");
		} catch (Exception e) {
			throw new JABException(e);
		}
	}

	public void rollback() throws JABException {
		log.debug("JABTransaction rollback");

		try {
			log.debug("calling rollback");
			terminator.rollback();
			active = false;
			ThreadActionData.popAction();
			log.debug("called rollback on terminator");
		} catch (Exception e) {
			throw new JABException(e);
		}
	}

	/**
	 * Add the specified thread to the list of threads associated with this
	 * transaction.
	 * 
	 * @return <code>true</code> if successful, <code>false</code> otherwise.
	 */
	public final boolean addChildThread(Thread t) {
		if (t == null)
			return false;

		synchronized (this) {
			// if (actionStatus <= ActionStatus.ABORTING)
			if (active) {
				if (_childThreads == null)
					_childThreads = new Hashtable();

				// TODO _childThreads.put(ThreadUtil.getThreadId(t), t); //
				// makes sure so we don't get duplicates

				return true;
			}
		}

		return false;
	}

	/**
	 * Remove a child thread. The current thread is removed.
	 * 
	 * @return <code>true</code> if successful, <code>false</code> otherwise.
	 */
	public final boolean removeChildThread() // current thread
	{
		return removeChildThread(ThreadUtil.getThreadId());
	}

	/**
	 * Remove the specified thread from the transaction.
	 * 
	 * @return <code>true</code> if successful, <code>false</code> otherwise.
	 */
	public final boolean removeChildThread(String threadId) {
		if (threadId == null)
			return false;

		synchronized (this) {
			if (_childThreads != null) {
				_childThreads.remove(threadId);
				return true;
			}
		}

		return false;
	}

	public final JABTransaction parent() {
		return null;
	}

	/**
	 * Suspend all transaction association from the invoking thread. When this
	 * operation returns, the thread will be associated with no transactions.
	 * 
	 * @return a handle on the current JABTransaction (if any) so that the
	 *         thread can later resume association if required.
	 */
	/*
	 * TODO public static final JABTransaction suspend() { JABTransaction curr =
	 * ThreadActionData.currentAction();
	 * 
	 * if (curr != null) ThreadActionData.purgeActions();
	 * 
	 * return curr; }
	 */
	/**
	 * Resume transaction association on the current thread. If the specified
	 * transaction is null, then this is the same as doing a suspend. If the
	 * current thread is associated with transactions then those associations
	 * will be lost.
	 * 
	 * @param JABTransaction
	 *            act the transaction to associate. If this is a nested
	 *            transaction, then the thread will be associated with all of
	 *            the transactions in the hierarchy.
	 * 
	 * @return <code>true</code> if association is successful,
	 *         <code>false</code> otherwise.
	 */
	/*
	 * TODO public static final boolean resume(JABTransaction act) { if (act ==
	 * null) suspend(); else ThreadActionData.restoreActions(act);
	 * 
	 * return true; }
	 */
}
