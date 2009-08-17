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
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
#include "ThreadLocalStorage.h"
#include "XAResourceManagerFactory.h"
#include "TxInitializer.h"
#include "OrbManagement.h"
#include "TxManager.h"
#include "AtmiBrokerEnv.h"
#include "ace/Thread.h"
//#include "AtmiBrokerMem.h"

#define TX_GUARD(cond) {    \
    FTRACE(txmlogger, "ENTER"); \
    if (!cond) {  \
        LOG4CXX_WARN(txmlogger, (char*) "protocol error: open: " << _isOpen << " transaction: " << getSpecific(TSS_KEY));   \
        return TX_PROTOCOL_ERROR;   \
    }}

namespace atmibroker {
    namespace tx {

log4cxx::LoggerPtr txmlogger(log4cxx::Logger::getLogger("TxLogManager"));
//SynchronizableObject* AtmiBrokerMem::lock = new SynchronizableObject();
TxManager *TxManager::_instance = NULL;

TxManager *TxManager::get_instance()
{
    // TODO add synchronization
    // lock->lock();
    if (_instance == NULL)
        _instance = new TxManager();

    // lock->unlock();
    return _instance;
}

void TxManager::discard_instance()
{
    FTRACE(txmlogger, "ENTER");
    // lock->lock();
    if (_instance != NULL) {
        delete _instance;
        _instance = NULL;
    }
    // lock->unlock();
}

TxManager::TxManager() :
    _whenReturn(TX_COMMIT_DECISION_LOGGED), _controlMode(TX_UNCHAINED), _timeout (0L), _isOpen(false)
{
    FTRACE(txmlogger, "ENTER");
    _connection = init_orb((char*) "ots");
    LOG4CXX_DEBUG(txmlogger, (char*) "new CONNECTION: " << _connection);
}

TxManager::~TxManager()
{
    FTRACE(txmlogger, "ENTER");
    if (_connection) {
        LOG4CXX_DEBUG(txmlogger, (char*) "deleting CONNECTION: " << _connection);
        shutdownBindings(_connection);
        delete _connection;
    }
}

CORBA_CONNECTION* TxManager::init_orb(char* name)
{
    FTRACE(txmlogger, "ENTER");
    register_tx_interceptors(name);
    return ::initOrb(name);
}

atmibroker::tx::TxControl *TxManager::currentTx(const char *msg)
{
    FTRACE(txmlogger, "ENTER");
    atmibroker::tx::TxControl *tx = NULL;

    if ((!_isOpen || (tx = (TxControl *) getSpecific(TSS_KEY)) == NULL || !tx->isActive(NULL, true)) && msg) {
        LOG4CXX_INFO(txmlogger, (char*) "protocol violation (" << msg << ") open="
            << _isOpen << " TSS_KEY=" << getSpecific(TSS_KEY));
    }

    return tx;
}

#if 0
int TxManager::mem_test(void)
{
#define T1 1
#if defined(T1)
    begin();
    rm_start(TMNOFLAGS);
    rm_end(TMSUCCESS);
    commit();
#elif defined(T2)
    xbegin();
    _xaRMFac.test(_connection, 100L);
//    rm_start(TMNOFLAGS);
//    rm_end(TMSUCCESS);
    xcommit();
#else
    try {
        CosTransactions::Control_ptr ctrl = _txfac->create(_timeout);
        CosTransactions::Terminator_var term = ctrl->get_terminator();

        TxControl *tx = new TxControl(ctrl, 0);
           setSpecific(TSS_KEY, tx);
        _xaRMFac.test(_connection, ctrl, 100L);
        delete tx;

        term->commit(true);

    //    CORBA::release(ctrl);

        LOG4CXX_INFO(txmlogger, (char*) "mem_test 1 ok");
        return TX_OK;
    } catch (CORBA::SystemException & e) {
        LOG4CXX_WARN(txmlogger, (char*) "mem_test ex: " << e._name() << " minor: " << e.minor());
        return TX_FAIL;
    }
#endif
    return TX_OK;
}
#endif

int TxManager::begin(void)
{
    TX_GUARD(_isOpen && !getSpecific(TSS_KEY));

    // start a new transaction
    CosTransactions::Control_ptr ctrl = _txfac->create(_timeout);

    if (CORBA::is_nil(ctrl)) {
        LOG4CXX_WARN(txmlogger, (char*) "begin: create returned nil control");
        return TX_ERROR;
    }

    TxControl *tx = new TxControl(ctrl, ACE_OS::thr_self());
    // associate the tx with the callers thread and enlist all open RMs with the tx
    int rc = TxManager::tx_resume(tx, TMNOFLAGS);

    if (rc != XA_OK) {
        // one or more RMs failed to start - roll back the transaction
        LOG4CXX_WARN(txmlogger, (char*) "begin: XA resume error: " << rc);
        try {
            CosTransactions::Terminator_var term = ctrl->get_terminator();

            if (!CORBA::is_nil(term))
                term->rollback();
        } catch (...) {
        }

        delete tx;

        return TX_ERROR;
    }

    LOG4CXX_DEBUG(txmlogger, (char*) "begin: ok");

    return TX_OK;
}

int TxManager::commit(void)
{
    FTRACE(txmlogger, "ENTER");
    return complete(true);
}

int TxManager::rollback(void)
{
    FTRACE(txmlogger, "ENTER");
    return complete(false);
}

int TxManager::rollback_only(void)
{
    TxControl *tx;

    TX_GUARD(((tx = currentTx("rollback_only")) != NULL));

    // inform the local resource managers
    rm_end(TMFAIL);
    return tx->rollback_only();
}

int TxManager::complete(bool commit)
{
    TxControl *tx;
    int outcome;

    TX_GUARD(((tx = currentTx("complete")) != NULL));

    // no need to call rm_end since each RM wrapper calls xa_end during the prepare call
    outcome = (commit ? tx->commit(reportHeuristics()) : tx->rollback());

    delete tx;

    return (isChained() ? chainTransaction(outcome) : outcome);
}

int TxManager::chainTransaction(int outcome)
{
    FTRACE(txmlogger, "ENTER");
    /*
     * NOTE: outcome will only truly represent the outcome of commit if the commit_return
     * characteristic is TX_COMMIT_COMPLETED (see method reportHeuristics()).
     * Using get Coordinator::get_status is ambiguous since NoTransaction can mean the
     * transaction committed or rolled back and has been forgotten.
     * TODO in begin register a participant in the transaction so we can definitively know
     * the transaction outcome.
     */
    switch(begin()) {
    case TX_OK:
        return TX_OK;
    default:
        switch (outcome) {
        case TX_OK:
            return TX_NO_BEGIN;
        case TX_ROLLBACK:
            return TX_ROLLBACK_NO_BEGIN;
        case TX_MIXED:
            return TX_MIXED_NO_BEGIN;
        case TX_HAZARD:
            return TX_HAZARD_NO_BEGIN;
        default:
            return outcome;
        }
    }
}

int TxManager::set_commit_return(COMMIT_RETURN when_return)
{
    TX_GUARD(_isOpen);

    if (when_return != TX_COMMIT_DECISION_LOGGED &&
        when_return != TX_COMMIT_COMPLETED)
        return TX_EINVAL;

    _whenReturn = when_return;

    return TX_OK;
}

int TxManager::set_transaction_control(TRANSACTION_CONTROL mode)
{
    TX_GUARD(_isOpen);

    if (mode != TX_UNCHAINED && mode != TX_CHAINED)
        return TX_EINVAL;

    _controlMode = mode;

    return TX_OK;
}

int TxManager::set_transaction_timeout(TRANSACTION_TIMEOUT timeout)
{
    TX_GUARD(_isOpen);

    if (timeout < 0)
        return TX_EINVAL;

    _timeout = timeout;

    return TX_OK;
}

int TxManager::info(TXINFO *info)
{
    TX_GUARD(_isOpen);

    atmibroker::tx::TxControl *tx = currentTx(NULL);

    if (info != 0) {
        info->transaction_state = -1L;
        (info->xid).formatID = -1L; // means the XID is null
        info->when_return = _whenReturn;
        info->transaction_control = _controlMode;
        /*
         * the timeout that will be used when this process begins the next transaction
         * (it is not neccessarily the timeout for the current transaction since
         * this may have been set in another process or it may have been changed by
         * this process after the current transaction was started).
         */
        info->transaction_timeout = _timeout;

        if (tx != NULL) {
            XAResourceManagerFactory::getXID(info->xid);
            info->transaction_state = tx->get_status();
        }
    }

    LOG4CXX_DEBUG(txmlogger, (char*) "info status=" << info->transaction_state << " tx=" << tx);
    return (tx != NULL ? 1 : 0);
}

int TxManager::open(void)
{
    FTRACE(txmlogger, "ENTER");

    if (_isOpen)
        return TX_OK;

    if (_txfac == NULL) {
        char *transFactoryId = AtmiBrokerEnv::get_instance()->getenv((char*)"TRANS_FACTORY_ID");

        if (transFactoryId == NULL || strlen(transFactoryId) == 0) {
            LOG4CXX_ERROR(txmlogger, (char*) "Please set the TRANS_FACTORY_ID env variable");
            return TX_ERROR;
        }

        try {
            CosNaming::NamingContextExt_ptr nce = (CosNaming::NamingContextExt_ptr) _connection->default_ctx;
            CosNaming::Name *name = nce->to_name(transFactoryId);
            LOG4CXX_DEBUG(txmlogger, (char*) "resolving Tx Fac Id: " << transFactoryId);
            CORBA::Object_var obj = nce->resolve(*name);
            delete name;
            LOG4CXX_DEBUG(txmlogger, (char*) "resolved OK: " << (void*) obj);
            _txfac = CosTransactions::TransactionFactory::_narrow(obj);
            LOG4CXX_DEBUG(txmlogger, (char*) "narrowed OK: " << (void*) _txfac);

        } catch (CORBA::SystemException & e) {
            LOG4CXX_ERROR(txmlogger, 
                (char*) "Error resolving Tx Service: " << e._name() << " minor code: " << e.minor());
            return TX_ERROR;
        } catch (...) {
            LOG4CXX_ERROR(txmlogger, 
                (char*) "Unknown error resolving Tx Service: " << transFactoryId);
            return TX_ERROR;
        }
    }

    if (rm_open() != 0) {
        LOG4CXX_ERROR(txmlogger, (char*) "At least one resource manager failed");
        (void) rm_close();

        return TX_ERROR;
    }

    // re-initialize values
    _controlMode = TX_UNCHAINED;
    _timeout = 0L;

    _isOpen = true;

    return TX_OK;
}

int TxManager::close(void)
{
    FTRACE(txmlogger, "ENTER");
    if (!_isOpen)
        return TX_OK;

    TX_GUARD((getSpecific(TSS_KEY) == NULL));

    _isOpen = false;
    rm_close();

    return TX_OK;
}

int TxManager::rm_open(void)
{
    FTRACE(txmlogger, "ENTER");
    try {
        _xaRMFac.createRMs(_connection);
        return 0;
    } catch (RMException& ex) {
        LOG4CXX_WARN(txmlogger, (char*) "failed to load RMs: " << ex.what());
        return -1;
    }
}

// private methods

void TxManager::rm_close(void)
{
    FTRACE(txmlogger, "ENTER");
    _xaRMFac.destroyRMs();
}

int TxManager::rm_end(int flags)
{
    FTRACE(txmlogger, "ENTER");
    return _xaRMFac.endRMs(flags);
}

int TxManager::rm_start(int flags)
{
    FTRACE(txmlogger, "ENTER");
    return _xaRMFac.startRMs(flags);
}

CosTransactions::Control_ptr TxManager::get_ots_control()
{
    FTRACE(txmlogger, "ENTER");
    TxControl *tx = (TxControl *) getSpecific(TSS_KEY);

    return (tx ? tx->get_ots_control() : 0);
}

int TxManager::tx_resume(CosTransactions::Control_ptr control, int creator, int flags)
{
    FTRACE(txmlogger, "ENTER");
    TxControl *tx = new TxControl(control, creator);
    int rc = TxManager::tx_resume(tx, flags);
    if (rc != XA_OK) {
        delete tx;
    }

    return rc;
}

int TxManager::tx_resume(char* ctrlIOR, char *orbname, int flags)
{
    FTRACE(txmlogger, "ENTER");
    CORBA::Object_ptr p = atmi_string_to_object(ctrlIOR, orbname);

    LOG4CXX_DEBUG(txmlogger, (char*) "tx_resume orb=" << orbname << (char *) " IOR=" << ctrlIOR << " ptr=" << p);

    if (!CORBA::is_nil(p)) {
        CosTransactions::Control_ptr cptr = CosTransactions::Control::_narrow(p);
        CORBA::release(p); // dispose of it now that we have narrowed the object reference

        return TxManager::tx_resume(cptr, 0, flags); // why 0 tid
    } else {
        LOG4CXX_WARN(txmlogger, (char*) "tx_resume: invalid control IOR: " << ctrlIOR);
    }

    return TMER_INVAL;
}

int TxManager::tx_resume(TxControl *tx, int flags)
{
    FTRACE(txmlogger, "ENTER " << tx << " - flags=" << std::hex << flags);
    int rc = XAER_NOTA;

    if (getSpecific(TSS_KEY)) {
        TxControl *pt = (TxControl *) getSpecific(TSS_KEY);

        LOG4CXX_WARN(txmlogger, (char *) "Thread already bound to " << pt << " (deleting it)");
        delete pt;
    }

    try {
        // TMJOIN TMRESUME TMNOFLAGS
        // must associate the tx with the thread before calling start on each open RM
           setSpecific(TSS_KEY, tx);
        if ((rc = TxManager::get_instance()->rm_start(flags)) == XA_OK) {
            LOG4CXX_DEBUG(txmlogger, (char *) "Resume tx: ok");

            return XA_OK;
        } else {
            LOG4CXX_WARN(txmlogger, (char *) "Resume tx: error: " << rc);
        }
    } catch (...) {
        LOG4CXX_WARN(txmlogger, (char *) "Resume tx: generic exception");
    }

    destroySpecific(TSS_KEY);

    return rc;
}

CosTransactions::Control_ptr TxManager::tx_suspend(int thr_id, int flags)
{
    FTRACE(txmlogger, "ENTER");
    return (tx_suspend((TxControl *) getSpecific(TSS_KEY), thr_id, flags));
}

/**
 * Suspend the transaction and return the control.
 * The caller is responsible for releasing the returned control
 */
CosTransactions::Control_ptr TxManager::tx_suspend(TxControl *tx, int thr_id, int flags)
{
    FTRACE(txmlogger, "ENTER");
    if (tx
        && tx->isActive(NULL, true)    // tx is active
//        && (thr_id == 0 || tx->thr_id() != (int) thr_id)    // not the owning thread or don't care about owners
        ) {
        // increment the control reference count
        CosTransactions::Control_ptr ctrl = tx->get_ots_control();
        // suspend all open Resource Managers (TMSUSPEND TMMIGRATE TMSUCCESS TMFAIL)
        (void) TxManager::get_instance()->rm_end(flags);
        // disassociate the transaction from the callers thread
        tx->suspend();
        delete tx;

        FTRACE(txmlogger, "< ctrl: " << ctrl);
        return ctrl;
    }

    FTRACE(txmlogger, "< ctrl: 0x0");
     return NULL;
}

} //    namespace tx
} //namespace atmibroker
