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

public class JtsTransactionImple extends TransactionImple
{
    private static final Logger log = LogManager
			.getLogger(JtsTransactionImple.class);
    private static TransactionManager tm;
    private static final String IORTag = "IOR";

//	private Transaction prev;

    public JtsTransactionImple (ControlWrapper wrapper)
    {
        super(new AtomicTransaction(wrapper));
    }
/*
	protected void setPreviousTx(Transaction tx)
	{
		prev = tx;
	}
	protected Transaction getPreviousTx()
	{
		return prev;
	}
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

    public static boolean hasTransaction()
    {
        try {
            return (getTransactionManager() != null && tm.getTransaction() != null);
        } catch (SystemException e) {
            return false;
        }
    }

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

    public static void resume(String ior)
    {
        log.info("resume control");
        Transaction tx = controlToTx(ior);
        resume(tx);
    }
/*
    public static void resume(String ior, Transaction prev)
    {
        log.info("resume control");
        Transaction tx = controlToTx(ior);
        resume(tx);

        if (tx instanceof JtsTransactionImple) {
            ((JtsTransactionImple) tx).prev = tx;
        }
    }

    public static Transaction suspend(boolean resumePrev)
    {
        Transaction tx = suspend();

        if (resumePrev && tx != null && (tx instanceof JtsTransactionImple)) {
            JtsTransactionImple t = (JtsTransactionImple) tx;

            if (t.prev != null)
                resume(t.prev);
        }

        return tx;
    }
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

    public static String getTransactionIOR()
    {
        log.info("getTransactionIOR");

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

    public static ControlWrapper createControlWrapper(String ior) {
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

}
