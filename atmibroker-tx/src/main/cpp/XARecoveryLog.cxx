#include <iostream>
#include <string>
#include <sstream>

#include "log4cxx/logger.h"

#include "XARecoveryLog.h"
#include "AtmiBrokerEnv.h"

// the persistent store for recovery records
static const char* DEF_RCLOG_NAME = "rclog";
static char RCLOGPATH[1024];

log4cxx::LoggerPtr xarcllogger(log4cxx::Logger::getLogger("TxLogXARecoveryLog"));

using namespace std;

// convert an X/Open XID to a string (used as a key into the allocator to retrieve the rrec_t data type
// associated with the key)
static string xid_to_string(XID& xid)
{
	std::stringstream out;

	out << xid.formatID << ':' << xid.gtrid_length << ':'<< xid.bqual_length << ':' << (char *) (xid.data + xid.gtrid_length);

	return out.str();
}

/*
// comparator for XID's
static int compareXids(const XID& xid1, const XID& xid2)
{
	char *x1 = (char *) &xid1;
	char *x2 = (char *) &xid2;
	char *e = (char *) (x1 + sizeof (XID));

	while (x1 < e)
		if (*x1 < *x2)
			return -1;
		else if (*x1++ > *x2++)
			return 1;

	return 0;
}
*/

// locate the path to the backing store for the recovery log
static void init_logpath(const char *fname)
{
	char *logfile = (fname == NULL ? ACE_OS::getenv("BLACKTIE_RECOVERY_LOG") : (char *) fname);

	if (logfile == NULL) {
		char *logdir = ACE_OS::getenv("BLACKTIE_CONFIGURATION_DIR");

		if (logdir != NULL)
			ACE_OS::snprintf(RCLOGPATH, sizeof (RCLOGPATH), "%s/%s", logdir, DEF_RCLOG_NAME);
		else
			ACE_OS::snprintf(RCLOGPATH, sizeof (RCLOGPATH), "%s", DEF_RCLOG_NAME);
	} else {
		ACE_OS::snprintf(RCLOGPATH, sizeof (RCLOGPATH), "%s", logfile);
	}
}

/*
 * Construct a recovery log for storing XID's and their associated OTS Recovery Records.
 * The (ACE) memory pool for the log is backed by a memory mapped file)
 */
XARecoveryLog::XARecoveryLog(const char* logfile) throw (RMException)
{
	ACE_MMAP_Memory_Pool_Options opts(ACE_DEFAULT_BASE_ADDR);
	init_logpath(logfile);

	LOG4CXX_TRACE(xarcllogger, (char *) "Using file " << RCLOGPATH);

	pool_ = new mmpool_t(RCLOGPATH, RCLOGPATH, &opts);

	if (pool_ == 0)
		throw new RMException("Error creating recovery log allocator", -1);
}

XARecoveryLog::~XARecoveryLog()
{
	LOG4CXX_TRACE(xarcllogger, (char *) "destructor");
	delete pool_;
}

/*
 * Insert a recovery record into persistent storage.
 * The bind call ensures that the record is synced to disk. [Asside:- any changes made to the
 * rrec_t data type after the bind are automatically synced to disk, the implementation does
 * use this feature.]
 */
int XARecoveryLog::add_rec(XID& xid, char *ior)
{
	// malloc enough space from the memory mapped recovery log for the requested branch
	// plus space to hold the IOR (include the string null terminator for easy debugging
	void* mp = pool_->malloc(sizeof (rrec_t) + strlen(ior) + 1);
	char* iorcpy = (char*) mp + sizeof (rrec_t);
	// the ace allocator keys its blocks by name - use the string form of the XID as the key
	string key = xid_to_string(xid);
	int rv = -1;

	LOG4CXX_TRACE(xarcllogger, (char *) "adding XID: " << key.c_str() << " IOR: " << ior);

	if(mp == 0) {
		LOG4CXX_ERROR(xarcllogger, (char *) "Out of memory");
	} else {
		ACE_OS::memcpy(iorcpy, ior, strlen(ior) + 1);

		// new the space for the rrec_t using the memory allocated from the allocator
		rrec_t* rrp = new (mp) rrec_t(xid, iorcpy);

		// and bind the name (the string form of the xid) to the allocated block
		if ((rv = pool_->bind(key.c_str(), rrp)) == -1) {
			LOG4CXX_ERROR(xarcllogger,  (char *) "Errory binding key: " << rv);
		} else {
			rv = 0;
		}
#ifdef TESTSYNC 
		// to test that the allocator synced the block after the bind generate
		// a segmentation fault. The block should be stored in the backing file
		// for the allocator so it should be present if a find_rec call is issued
		// on restart.
		if (strcmp(ior, "SEGV") == 0) {
			char *p = 0;
			*p = 0;
		}
#endif
	}

	return rv;
}

// locate a recovery record by XID
rrec_t*  XARecoveryLog::find_rec(XID& xid)
{
    string s = xid_to_string(xid);
    return find_rec(s.c_str());
}

// locate a recovery record by binding name (in this implementation the name
// is the string form of the XID
rrec_t* XARecoveryLog::find_rec(const char *name)
{
	void *rec;

//  use an iterator to find by anything other than name or XID
//  mmpool_iter_t i(*pool_);
//  for (void *rec = 0; i.next(rec) != 0; i.advance ()) {
//	  rrec_t *rrp = reinterpret_cast<rrec_t *> (rec);
//  }
	if (pool_->find(name, rec) == -1)
		return 0;
	else
		return reinterpret_cast<rrec_t *> (rec);
}

int XARecoveryLog::del_rec(XID& xid)
{
	string s = xid_to_string(xid);

	LOG4CXX_TRACE(xarcllogger, (char *) "deleting XID: " << s.c_str());
	return del_rec(s.c_str());
}

// unbinding the name of a block is the delete operation
int XARecoveryLog::del_rec(const char *name)
{
	if (pool_->unbind(name) != -1)
		return 0;

	return -1;
}

/*
 * Obtain an iterator for the log. Note that this implementation uses ACE_Null_Mutex
 * to protect the log - ie there is now protection for concurrent threads modifying the
 * same record (see the XARecoveryLog header for the allocator definition if there is a
 * requirement for protecting access to the log). 
 */
void* XARecoveryLog::aquire_iter()
{
	return new mmpool_iter_t(*pool_);
}

// the aquire_iter operation returns an handle that consumes memory so must be released
// when done.
void XARecoveryLog::release_iter(void *iter)
{
	if (iter)
		delete reinterpret_cast<mmpool_iter_t *> (iter);
}

rrec_t* XARecoveryLog::next(void *iter)
{
	if (iter) {
		mmpool_iter_t* i = reinterpret_cast<mmpool_iter_t *> (iter);
		void *rec = 0;

		if (i->next(rec)) {
			i->advance();
			return reinterpret_cast<rrec_t *> (rec);
		}
	}

	return 0;
}
