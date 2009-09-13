#ifndef _XARECOVERYLOG_H
#define _XARECOVERYLOG_H

#include "xa.h"
#include "atmiBrokerTxMacro.h"
#include "RMException.h"

#include "ace/MMAP_Memory_Pool.h"
#include "ace/Malloc_T.h"
#include "ace/PI_Malloc.h"
#include "ace/Null_Mutex.h"

/*
 * Simple log for recording branch recovery coordinators at
 * prepare time, for deleting them after commit and for
 * reading outstanding branches after a crash.
 *
 * At startup time XAResourceFactory has exclusive access to the
 * log (since at startup there are no XAResourceAdaptorImpl instances).
 *
 * If locking is required use a non null ACE mutex (see definition of mmpool_t below)
 */
typedef ACE_Malloc_T <ACE_MMAP_MEMORY_POOL, ACE_Null_Mutex, ACE_PI_Control_Block> mmpool_t;
typedef ACE_Malloc_LIFO_Iterator_T <ACE_MMAP_MEMORY_POOL, ACE_Null_Mutex, ACE_PI_Control_Block> mmpool_iter_t;

// a recovery record consists of an XID and a CosTransactions RecoveryCoordinator IOR
class rrec_t
{
public:
	rrec_t(XID& xid, char* ior) :xid_(xid), ior_(ior) {}

	char* ior() { return ior_; }
	XID& xid() { return xid_; }

private:
	XID xid_;
	ACE_Based_Pointer_Basic<char> ior_;
};

// memory mapped log of rrec_t with automatic sync to backing storage
class BLACKTIE_TX_DLL XARecoveryLog {
public:
	XARecoveryLog(const char *logfile = 0) throw (RMException);
	~XARecoveryLog();

	// add, lookup and delete log records
	int add_rec(XID& xid, char *ior);
	rrec_t* find_rec(const char *name);
	rrec_t* find_rec(XID& xid);
	int del_rec(const char *name);
	int del_rec(XID& xid);

	// mechanism for iterating through the log
	void* aquire_iter();
	void release_iter(void *iter);
	rrec_t* next(void *iter);

private:
	mmpool_t* pool_;
};

#endif //_XARECOVERYLOG_H
