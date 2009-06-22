package org.jboss.blacktie.jatmibroker.transport;

import javax.transaction.*;
import javax.naming.Context;
import javax.naming.NamingException;
import javax.naming.InitialContext;

import org.omg.CosTransactions.Control;
import org.omg.CosTransactions.Unavailable;

import com.arjuna.ats.internal.jta.transaction.jts.TransactionImple;
import com.arjuna.ats.internal.jta.transaction.jts.AtomicTransaction;
import com.arjuna.ats.internal.jts.ControlWrapper;
import com.arjuna.ats.internal.jts.ORBManager;

import org.jboss.blacktie.jatmibroker.jab.JABTransaction;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

/**
 * Wrapper class for running JBossTS transactions in an application server
 */
public class JtsTransactionImple extends TransactionImple
{
    private static final Logger log = LogManager
			.getLogger(JtsTransactionImple.class);
    private static TransactionManager tm;
    private static final String IORTag = "IOR";

    /**
     * Construct a transaction based on an OTS control
     * @param wrapper the wrapped OTS control
     */
    public JtsTransactionImple (ControlWrapper wrapper)
    {
        super(new AtomicTransaction(wrapper));
    }

    /**
     * associate a new transaction with the calling thread
     * @return true if a new transaction is associated
     */
    public static boolean begin() {
        if (getTransactionManager() != null) {
            try {
                tm.begin();
                return true;
            } catch (NotSupportedException e) {
                log.debug("Unable to start a new transaction: " + e);
            } catch (SystemException e) {
                log.debug("Unable to start a new transaction: " + e);
            }
        }

        return false;
    }

    /**
     * check whether the calling thread is associated with a transaction
     * @return true if there is transaction on the callers thread
     */
    public static boolean hasTransaction()
    {
        try {
            return (getTransactionManager() != null && tm.getTransaction() != null);
        } catch (SystemException e) {
            return false;
        }
    }

    /**
     * Re-associate a transaction with the callers thread
     * @param tx the transaction to associated
     */
    public static void resume(Transaction tx)
    {
        try {
            if (tx != null && getTransactionManager() != null) {
                log.info("resuming " + tx);

                tm.resume(tx);
            } else {
        		log.info("nothing to rusume: tm=" + tm);
			}
        } catch (SystemException e) {
       		log.info("resume error: " + e);
        } catch (InvalidTransactionException e) { // invalid transaction.
       		log.info("resume error: " + e);
        } catch (IllegalStateException e) { // the thread is already associated with another transaction.
       		log.info("resume error: " + e);
        }
    }

    /**
     * Associated a transaction with the callers thread
     * @param ior IOR for the corresponding OTS transaction
     */
    public static void resume(String ior)
    {
        log.info("resume control");
        Transaction tx = controlToTx(ior);
        resume(tx);
    }

    /**
     * Dissassociate the transaction currently associated with the callers thread
     * @return the dissassociated transaction
     */
    public static Transaction suspend()
    {
        log.info("suspend");

        try {
            if (getTransactionManager() != null) {
        		log.info("suspending current");
                return tm.suspend();
			}
        } catch (SystemException e) {
       		log.info("suspend error: " + e);
        }

        return null;
    }

    /**
     * Convert an IOR representing an OTS transaction into a JTA transaction
     * @param ior the CORBA reference for the OTS transaction
     * @return a JTA transaction that wraps the OTS transaction
     */
    private static Transaction controlToTx(String ior)
    {
        log.info("controlToTx: ior: " + ior);

        if (ior == null)
            return null;

        ControlWrapper cw = createControlWrapper(ior);
        TransactionImple tx = (TransactionImple) TransactionImple.getTransactions().get(cw.get_uid());

        if (tx == null) {
        	log.info("controlToTx: creating a new tx - wrapper: " + cw);
            tx = new JtsTransactionImple(cw);
            putTransaction(tx);
        }

        return tx;
    }

    /**
     * Lookup the JTA transaction manager
     * @return the JTA transaction manager in the VM
     */
    public static TransactionManager getTransactionManager()
    {
        if (tm == null) {
            try {
                Context ctx = new InitialContext();
                tm = (TransactionManager) ctx.lookup("java:/TransactionManager");
            } catch (NamingException e) {
                return null;
            }
        }

        return tm;
    }

    /**
     * If the current transaction represents an OTS transaction then return it IOR
     * @return the IOR or null if the current transaction is not an OTS transaction
     */
    public static String getTransactionIOR()
    {
        log.debug("getTransactionIOR");

        JABTransaction curr = JABTransaction.current();
        if (curr != null) {
        	log.info("have JABTransaction");
            return curr.getControlIOR();
        } else if (getTransactionManager() != null) {
            try {
        		log.debug("have tx mgr");
                Transaction tx = tm.getTransaction();

                if (tx instanceof TransactionImple) {
        			log.info("have arjuna tx");
                    TransactionImple atx = (TransactionImple) tx;
                    ControlWrapper cw = atx.getControlWrapper();

                    if (cw == null) {
                        log.warn("getTransactionIOR transaction has no control wrapper");
					} else {
                        try {
                        	log.debug("lookup control");
                            Control c = cw.get_control();
                            String ior = ORBManager.getORB().orb().object_to_string(c);

                            log.debug("getTransactionIOR: ior: " + ior);
                            
                            return ior;
                        } catch (Unavailable e) {
                            log.warn("getTransactionIOR transaction has no control: " + e.getMessage());
                        }
                    }
                }
            } catch (SystemException e) {
                log.debug("excpeption: " + e.getMessage());
                e.printStackTrace();
            }
        }

        return null;
    }

    private static ControlWrapper createControlWrapper(String ior) {
        if  (ior.startsWith(IORTag)) {
            org.omg.CORBA.Object obj = ORBManager.getORB().orb().string_to_object(ior);
            Control control =  org.omg.CosTransactions.ControlHelper.narrow(obj);

            if (control == null)
                log.debug("createProxy: ior not a control");

            return new ControlWrapper(control);
        } else {
            return null;
        }
    }

	private static org.omg.CORBA.ORB getDefaultORB() {
        try {
            return ORBManager.getORB().orb();
        } catch (Throwable t) {
            return null;
        }
    }
}
