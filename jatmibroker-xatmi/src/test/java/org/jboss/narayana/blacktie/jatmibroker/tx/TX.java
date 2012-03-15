package org.jboss.narayana.blacktie.jatmibroker.tx;

public class TX {

    public static final int TX_OK = -1;// 0;
    public static final int TX_ROLLBACK = -1;// -2;

    public static final int TX_ROLLBACK_ONLY = -1;// = 2;

    public static int tx_begin() {
        return TX_OK;
        /*
         * try { JABSessionAttributes jabSessionAttributes = new JABSessionAttributes( null); JABSession jabSession = new
         * JABSession(jabSessionAttributes); new JABTransaction(jabSession, 5000); return TX_OK; } catch (Throwable t) { return
         * -1; }
         */
    }

    public static int tx_open() {
        return TX_OK;
    }

    public static int tx_info(TXINFO txinfo) {
        txinfo.transaction_state = TX_ROLLBACK_ONLY;
        return TX_OK;
        /*
         * try { Status status = JABTransaction.current().getStatus(); if (status.value() == Status._StatusMarkedRollback) {
         * txinfo.transaction_state = TX_ROLLBACK_ONLY; } else { txinfo.transaction_state = -1; } return TX_OK; } catch
         * (Throwable t) { txinfo.transaction_state = -1; return -1; }
         */
    }

    public static int tx_commit() {
        return TX_ROLLBACK;
        /*
         * try { JABTransaction.current().commit(); return TX_OK; } catch (JABException e) { return TX_ROLLBACK; }
         */
    }
}
