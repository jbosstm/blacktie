#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"

//#include <db.h>
#include <iostream>

#include "AtmiBrokerEnv.h"
#include "txi.h"

#include "XARecoveryLog.h"

log4cxx::LoggerPtr xarcllogger(log4cxx::Logger::getLogger("TxLogXARecoverLog"));

extern std::ostream& operator<<(std::ostream &os, const XID& xid);

/*
static int putrec(DB *dbp, const void *k, size_t ksz, const void *v, size_t vsz)
{
	DBT key, data; 
	int ret;

	memset(&key, 0, sizeof (key));
	key.data = (void *) k;
	key.size = ksz;
	memset(&data, 0, sizeof (data));
	data.data = (void *) v;
	data.size = vsz;

	if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) == 0) {
		LOG4CXX_TRACE(xarcllogger, "db op put: key: " << (char *) key.data <<
			" value: " << (char *)data.data << " vsz: " << vsz); 
	} else {
		LOG4CXX_WARN(xarcllogger, " db op put: " << db_strerror(ret)); 
	} 

	return ret;
}

static int getrec(DB *dbp, const void *k, size_t ksz, DBT *data)
{
	DBT key; 
	int ret;

	memset(&key, 0, sizeof(key)); 
	key.data = (void *) k; 
	key.size = ksz;

	if ((ret = dbp->get(dbp, NULL, &key, data, 0)) == 0) {
		LOG4CXX_TRACE(xarcllogger, "db op get: key: " << (char *) key.data <<
			" value: " << (char *)data->data << " vsz: " << data->size);
	} else {
		LOG4CXX_WARN(xarcllogger, "db op get: " << db_strerror(ret)); 
	}

	return ret;
}

static int delrec(DB *dbp, const void *k, size_t ksz)
{
	DBT key; 
	int ret;

	memset(&key, 0, sizeof(key)); 
	key.data = (void *) k; 
	key.size = ksz;

	if ((ret = dbp->del(dbp, NULL, &key, 0)) == 0) {
		LOG4CXX_TRACE(xarcllogger, "db op del: key: " << (char *) k); 
	} else {
		LOG4CXX_WARN(xarcllogger, "db op del: key: " << (char *) k <<
			" error: " << db_strerror(ret)); 
	} 

	return ret;
}

static int closedb(DB *dbp, int ret)
{
	int rv;

	if ((rv = dbp->close(dbp, 0)) != 0 && ret == 0) {
		LOG4CXX_WARN(xarcllogger, "db op close: " << db_strerror(rv)); 
	    return rv; 
	}

	LOG4CXX_TRACE(xarcllogger, "db op close ok"); 
	return (ret); 
}

static int opendb(DB **dbp, const char *dbfile)
{
	int ret;

	if ((ret = db_create(dbp, NULL, 0)) != 0) {
		LOG4CXX_WARN(xarcllogger, "db op create: " << db_strerror(ret)); 
		return (1); 
	} 

	if ((ret = (*dbp)->open(*dbp, NULL, dbfile, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
		LOG4CXX_WARN(xarcllogger, "db op open " << dbfile << " error: " << db_strerror(ret)); 
		return closedb(*dbp, ret);
	}

	return 0;
}

static int copy_data(DBT& data,  void **rc)
{
	if ((*rc = malloc(data.size)) == NULL) {
		LOG4CXX_ERROR(xarcllogger, "XARecoverLog: out of memory"); 
		return -1;
	}

	memcpy(*rc, data.data, data.size);

	return 0;
}
*/
#define DEF_RCLOG_NAME	"rclog"

XARecoveryLog::XARecoveryLog(const char *dbfile) throw (RMException)
{
	char RCLOGPATH[1024];
//	int ret;
	char *logfile = (dbfile == NULL ? ACE_OS::getenv("BLACKTIE_RECOVERY_LOG") : (char *) dbfile);

	FTRACE(xarcllogger, "ENTER logfile: " << logfile);

	if (logfile == NULL) {
		char *logdir = ACE_OS::getenv("BLACKTIE_CONFIGURATION_DIR");

		if (logdir != NULL)
			ACE_OS::snprintf(RCLOGPATH, sizeof (RCLOGPATH), "%s/%s", logdir, DEF_RCLOG_NAME);
		else
			ACE_OS::snprintf(RCLOGPATH, sizeof (RCLOGPATH), "%s", DEF_RCLOG_NAME);
	} else {
		ACE_OS::snprintf(RCLOGPATH, sizeof (RCLOGPATH), "%s", logfile);
	}

	LOG4CXX_DEBUG(xarcllogger, "Opening recovery log: " << RCLOGPATH); 

//	if ((ret = opendb((DB **) &dbp_, RCLOGPATH)) != 0) {
//		(void) closedb((DB *)dbp_, 0);
//		throw new RMException("Error opening recovery log", ret);
//	}

	FTRACE(xarcllogger, "ENTER dumping logfile: " << RCLOGPATH);
//	dump();
}

XARecoveryLog::~XARecoveryLog()
{
	FTRACE(xarcllogger, "ENTER");
//TODO	(void) closedb((DB *)dbp_, 0);
}

int XARecoveryLog::add(const XID& xid, const void* recoveryCoordinator, size_t vsz)
{
	FTRACE(xarcllogger, "ENTER adding xid: " << xid);
#if 0
opendb((DB **) &dbp_, "rclog");
	int rc = putrec((DB *)dbp_, &xid, sizeof (xid), recoveryCoordinator, vsz);
closedb((DB *)dbp_, 0);
return rc;
#endif
return 0;
}

int XARecoveryLog::get(const XID& xid, void **rc)
{
#if 0
	DBT data;
	int ret;

	FTRACE(xarcllogger, "ENTER geting xid: " << xid);
opendb((DB **) &dbp_, "rclog");
	memset(&data, 0, sizeof(data)); 
	if ((ret = getrec((DB *)dbp_, &xid, sizeof (xid), &data)) == 0)
		copy_data(data, rc);

closedb((DB *)dbp_, 0);
	return ret;
#endif
return 0;
}

int XARecoveryLog::del(const XID& xid)
{
	FTRACE(xarcllogger, "ENTER deleting xid: " << xid);
#if 0
opendb((DB **) &dbp_, "rclog");
	int ret = delrec((DB *)dbp_, &xid, sizeof (xid));

	if (ret != 0)
		dump();

closedb((DB *)dbp_, 0);
	return ret;
#endif
return 0;
}

int XARecoveryLog::erase_all()
{
#if 0
	XID* xid;
	char* rc;
	void* cursor;
	FTRACE(xarcllogger, "ENTER dumping recovery records ...");
	cursor_begin(&cursor);

	while (cursor_next(cursor, (void**) &xid, (void**) &rc) == 0) {
		LOG4CXX_DEBUG(xarcllogger, "erasing xid: "<< *xid << " value: " << rc);
		(void) delrec((DB *)dbp_, xid, sizeof (*xid));
		free(xid);
		free(rc);
	}

	cursor_end(cursor);
#endif
	return 0;
}

int XARecoveryLog::cursor_begin(void **cursor) {
#if 0
	int ret;
	DBC* dbcp;
	DB* dbp = (DB *)dbp_;

opendb((DB **) &dbp_, "rclog");
dbp = (DB *)dbp_;
	if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) == 0)
		*cursor = dbcp;

	return ret;
#endif
return 0;
}

int XARecoveryLog::cursor_next(void* cursor, void** kv, void** dv) {
#if 0
	int ret;
	DBC *dbcp = (DBC *) cursor;
	DBT key, val;

	memset(&key, 0, sizeof(key)); 
	memset(&val, 0, sizeof(val)); 
	if ((ret = dbcp->get(dbcp, &key, &val, DB_NEXT)) == 0) {
		copy_data(key, (void **) kv);
		copy_data(val, (void **) dv);
	}

	return ret;
#endif
return 0;
}

int XARecoveryLog::cursor_end(void* cursor) {
#if 0
	int ret;
	DBC *dbcp = (DBC *) cursor;

	if ((ret = dbcp->c_close(dbcp)) != 0) {
		LOG4CXX_WARN(xarcllogger, " cursor close: " << db_strerror(ret)); 
    }

closedb((DB *)dbp_, 0);
	return ret;
#endif
return 0;
}

int XARecoveryLog::dump() {
#if 0
	XID* xid;
	char* rc;
	void* cursor;

	FTRACE(xarcllogger, "ENTER dumping recovery records ...");
	cursor_begin(&cursor);

	while (cursor_next(cursor, (void**) &xid, (void**) &rc) == 0) {
		LOG4CXX_DEBUG(xarcllogger, "xid: "<< *xid << " value: " << rc);
		free(xid);
		free(rc);
	}

	cursor_end(cursor);

	return 0;
#endif
return 0;
}
