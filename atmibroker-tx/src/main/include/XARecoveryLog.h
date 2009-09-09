#ifndef _XARECOVERYLOG_H
#define _XARECOVERYLOG_H

#include "xa.h"
#include "atmiBrokerTxMacro.h"
#include "RMException.h"

/*
 * Simple log for recording branch recovery coordinators at
 * prepare time, for deleting them after commit and for
 * reading outstanding branches after a crash.
 *
 * The assumption is: each xid corresponds to a branch
 * as implemented by XAResourceAdaptorImpl and so multi-threaded
 * access to a record will not happen.
 *
 * At startup time XAResourceFactory has exclusive access to the
 * log (since at startup there are no XAResourceAdaptorImpl instances).
 *
 * BTW this implmentation still needs to be transactional since the log
 * records need to be synced to disk. The current implementation opens
 * and closes the underlying database on every update (that's why lots
 * of the code in the implementation file is commented out and temporary
 * code inserted.
 *
 * TODO I'll make it transactional in the next svn commit
 * For now it is just POC whilst I figure out how to support
 * persistent IORs in TAO which does not have an Implementation Repository.
 * Maybe we can coerce TOA to use Jacorb's Implementation Repository.
 */
class BLACKTIE_TX_DLL XARecoveryLog {
public:
	XARecoveryLog(const char * = NULL) throw (RMException);
	virtual ~XARecoveryLog();

	int add(const XID& xid, const void *rc_ref, size_t vsz);
	// get the data associated with an xid - caller must free rc_ref
	int get(const XID& xid, void **rc_ref);
	int del(const XID& xid);

	// methods for iterating through the log
	int cursor_begin(void **cursor);
	// get the next xid and data a cursor - caller must free kv and dv
	int cursor_next(void* cursor, void** kv, void** dv);
	int cursor_end(void* cursor);
	int erase_all();

	int dump();

private:
	//SynchronizableObject lock;
	void *dbp_;
};

#endif //_XARECOVERYLOG_H
