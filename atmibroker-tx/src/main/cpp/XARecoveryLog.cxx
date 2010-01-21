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
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <errno.h>

#include "log4cxx/logger.h"

#include "xa.h"
#include "ace/OS_NS_string.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_stdlib.h"

#include "XARecoveryLog.h"
#include "AtmiBrokerEnv.h"

#define INUSE 0xaf12L	// marker to indicate that the block is allocated
#define NBLOCKS 0x100	// the minimum number of blocks to allocate when expanding the arena
#define MAXBLOCKS   "0x1000"	// limit the size of the arena to this many blocks
#define IOR(rr) ((char *) (rr + 1))	// extract the IOR from a recovery record

#ifdef TESTSYNCDEL
#define SEGVONDEL()	{char *p = 0; *p = 0;}
#else
#define SEGVONDEL()	{}
#endif

#ifdef TESTSYNCADD
#define SEGVONADD(ior)	{if (strcmp(ior, "SEGV") == 0) {char *p = 0; *p = 0;}}
#else
#define SEGVONADD(ior)	{}
#endif

// the persistent store for recovery records
static const char* DEF_LOG = "rclog";
static char RCLOGPATH[1024];

log4cxx::LoggerPtr xarcllogger(log4cxx::Logger::getLogger("TxLogXARecoveryLog"));

using namespace std;

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

// convert an X/Open XID to a string (used as a key into the allocator to retrieve the rrec_t data type
// associated with the key)
static string xid_to_string(XID& xid)
{
	std::stringstream out;

	out << xid.formatID << ':' << xid.gtrid_length << ':'<< xid.bqual_length << ':' << (char *) (xid.data + xid.gtrid_length);

	return out.str();
}

/**
 * locate the path to the backing store for the recovery log
 */
static void init_logpath(const char *fname)
{
	if (fname) {
		ACE_OS::snprintf(RCLOGPATH, sizeof (RCLOGPATH), "%s", fname);
	} else {
		// if fname is not passed see if the log name is set in the environent
		AtmiBrokerEnv* env = AtmiBrokerEnv::get_instance();
		const char *rcLog    = env->getenv((char*) "BLACKTIE_RC_LOG_NAME", DEF_LOG);
		const char *servName = env->getenv((char*) "BLACKTIE_SERVER_NAME", rcLog);
		ACE_OS::snprintf(RCLOGPATH, sizeof (RCLOGPATH), "%s", servName);
		AtmiBrokerEnv::discard_instance();
	}

	LOG4CXX_TRACE(xarcllogger, (char *) "Using log file " << RCLOGPATH);
}

/*
 * Construct a recovery log for storing XID's and their associated OTS Recovery Records.
 * The log is backed by a file.
 */
XARecoveryLog::XARecoveryLog(const char* logfile) throw (RMException) :
	arena_(0), nblocks_((size_t) 0), maxblocks_(0)
{
	bool isClient = false;
	init_logpath(logfile);

	if (!isClient && !load_log(RCLOGPATH)) {
		LOG4CXX_ERROR(xarcllogger, (char *) "Error creating recovery log");
		//throw new RMException("Error creating recovery log ", -1);
		//TODO propagate the exception
	}
}

/**
 * free the arena and close the backing file
 */
XARecoveryLog::~XARecoveryLog()
{
	LOG4CXX_TRACE(xarcllogger, (char *) "destructor");
	if (arena_)
		ACE_OS::free(arena_);

	if (log_.is_open())
		log_.close();
}

/**
 * Read a collection of recovery records from file and load them into memory
 */
bool XARecoveryLog::load_log(const char* logname)
{
	LOG4CXX_TRACE(xarcllogger, (char *) "Loading log file: " << logname);
	AtmiBrokerEnv* env = AtmiBrokerEnv::get_instance();
	const char* maxblk = env->getenv("BLACKTIE_MAX_RCLOG_SIZE", MAXBLOCKS);
	AtmiBrokerEnv::discard_instance();

	ios_base::openmode mode = ios::out | ios::in | ios::binary;

	// make sure the log file exists
	FILE *fp = fopen(logname, "a+");

	if (fp == NULL) {
		LOG4CXX_ERROR(xarcllogger, (char *) "log open failed: " << errno);
		return false;
	}

	(void) fclose(fp);

	//log_.open (logname, mode | ios::app);
	//log_.close();
	log_.open (logname, mode);

	if (!log_.is_open()) {
		LOG4CXX_ERROR(xarcllogger, (char *) "log open failed for: " << logname);
		return false;
	}

	// calculate the number of bytes in the file
	size_t sz;
	fstream::pos_type s = log_.tellg();

	log_.seekg (0, ios::end);
	sz = (size_t) (log_.tellg() - s);

	if ((maxblocks_ = ACE_OS::strtol(maxblk, NULL, 0)) == 0) {
		LOG4CXX_ERROR(xarcllogger, (char *) "the env variable BLACKTIE_MAX_RCLOG_SIZE is invalid: " << (char *) maxblk);
		return false;
	}

	LOG4CXX_TRACE(xarcllogger, (char *) "recovery log size: " << maxblocks_);

	// allocate enough space into the arena for sz bytes
	if (!morecore(sz / sizeof (rrec_t) + 1, false))
		return false;

	// read the bytes from the file into the arena
	log_.seekg(0, ios::beg);
	log_.read((char *) arena_, sz);

//	debug_dump(arena_, arena_ + nblocks_);
	// verify that the log is consistent (must be called
	// at log creation time)
	check_log();

//	LOG4CXX_TRACE(xarcllogger, (char *) "load_log: contents:");
//	for (rrec_t* rr = find_next(0); rr; rr = find_next(rr))
//		LOG4CXX_TRACE(xarcllogger, (char *) "next: " << IOR(rr));

	return true;
}

/**
 * Increase the size of the arena for storing more records.
 * The size of the storage area is increased on demand but is never
 * reduced. If the backing file ends up containing large amounts of
 * free space it may be beneficial to reduce its size periodically.
 */
bool XARecoveryLog::morecore(size_t nblocks, bool dosync) {
	size_t nsz;
	rrec_t* ar;

	if (!log_.is_open())
		return false;

	if (nblocks < NBLOCKS)
		nblocks = NBLOCKS;

	nsz = nblocks_ + nblocks;

	if (nblocks_ > maxblocks_) {
		LOG4CXX_WARN(xarcllogger, (char *) "recovery log has grown beyond its configurable limit\n");
	} else if ((ar = (rrec_t*) ACE_OS::calloc(sizeof (rrec_t), nsz)) == 0) {
		LOG4CXX_WARN(xarcllogger, (char *) "recovery log: out of memory");
	} else {
		LOG4CXX_TRACE(xarcllogger, (char *) "increasing rc log blocks from " << hex << nblocks_ << " to " << nsz);

		ACE_OS::memcpy(ar, arena_, nblocks_);

		if (dosync)
			sync_rec(ar + nblocks_, nblocks * sizeof (rrec_t));

		nblocks_ = nsz;
		ACE_OS::free(arena_);
		arena_ = ar;

		return true;
	}

	return false;
}

void XARecoveryLog::sync_rec(void* p, size_t sz) {
	fstream::pos_type pos = (fstream::pos_type) ((char *) p - (char *) arena_);

	LOG4CXX_TRACE(xarcllogger, (char *) "sync " << p << " p size " << sz << " at pos " << (long) pos);
	log_.seekg(pos);
	log_.write((char *) p, sz);
	log_.sync();
}

rrec_t* XARecoveryLog::next_rec(rrec_t* p) {
	return (p->next && p->next < nblocks_ ?  arena_ + p->next : 0);
}

int XARecoveryLog::del_rec(XID& xid) {
	rrec_t* prev;
	rrec_t* next;

	if (!log_.is_open())
		return -1;

	lock_.lock();
	if (find(xid, &prev, &next) != 0) {
		LOG4CXX_TRACE(xarcllogger, (char *) "del_rec: xid " << xid_to_string(xid).c_str() << " not found");
		lock_.unlock();
		return -1;
	}

	// mark the block as free and sync it
	LOG4CXX_TRACE(xarcllogger, (char *) "deleting xid " << xid_to_string(xid).c_str() <<
		" at offset " << next->next << " IOR: " << IOR(next));
	next->magic = 0L;
	sync_rec(&(next->magic), sizeof (next->magic));

	// a failure here will leave prev pointing to a free block
	// which will be fixed up when the log is re-read
	SEGVONDEL();

	prev->next = next->next;
	sync_rec(&(prev->next), sizeof (prev->next));
	lock_.unlock();

	return 0;
}

const char* XARecoveryLog::get_ior(rrec_t& rr) {
	return IOR(&rr);
}

/**
 * Locate the next record following the passed in record.
 * If from is NULL the first record is returned - thus
 * the returned record also serves as a handle for finding
 * the next record - including the case where the record is
 * deleted. Note that it will find records inserted after
 * the handle but not ones inserted before it.
 */
rrec_t* XARecoveryLog::find_next(rrec_t* from) {
	if (!log_.is_open())
		return 0;

	if (from == 0) {
		if (arena_->magic == INUSE)
			return arena_;
		from = arena_;
	}

	for (rrec_t* p = next_rec(from); p; p = next_rec(p))
		if (p->magic == INUSE)
			return p;

	return 0;
}

/**
 * Locate the record keyed by xid. The returned record is passed
 * back to the caller in next (the record preceding it is returned
 * in prev - use for deleting records).
 */
int XARecoveryLog::find(XID xid, rrec_t** prev, rrec_t** next) {
	if (!log_.is_open())
		return -1;

	*prev = arena_;
	*next = arena_;

	while (*next) {
		if (compareXids(xid, (*next)->xid) == 0)
			return 0;

		*prev = *next;
		*next = next_rec(*next);
	}

	return -1;
}

/**
 * Lookup the IOR associated with the given XID
 */
char* XARecoveryLog::find_ior(XID& xid) {
	rrec_t* prev;
	rrec_t* next;

	if (find(xid, &prev, &next) == 0)
		return IOR(next);

	return 0;
}

/**
 * Insert a recovery record into persistent storage.
 */
int XARecoveryLog::add_rec(XID& xid, char* ior) {
	size_t nblocks = ((sizeof (rrec_t) + strlen(ior)) / sizeof (rrec_t)) + 1;
	rrec_t* fb; // next free block of the correct size

	LOG4CXX_TRACE(xarcllogger, (char *) "looking for a block of size " << nblocks * sizeof (rrec_t));

	if (!log_.is_open())
		return -1;

	lock_.lock();
	if ((fb = next_free(nblocks)) == 0) {
		LOG4CXX_TRACE(xarcllogger, (char *) "\tno space, increasing arena\n");

		if (!morecore(nblocks, true) || (fb = next_free(nblocks)) == 0) {
			LOG4CXX_ERROR(xarcllogger, (char *) "\tno large enough free region (required " <<
				nblocks << " blocks)");
			lock_.unlock();
			return -1;
		}
	}

	fb->xid = xid;
	fb->next = (fb - arena_) + nblocks; // nblocks is in rrec_t units
	fb->magic = INUSE;
	ACE_OS::memcpy(IOR(fb), ior, strlen(ior) + 1);
	LOG4CXX_TRACE(xarcllogger, (char *) "adding xid " << xid_to_string(xid).c_str() <<
		" at offset " << fb->next << ": IOR: " << ior);
	sync_rec(fb, nblocks * sizeof (rrec_t));


	// to test that the allocator synced the block correctly generate
	// a segmentation fault - the record should be available on restart.
	SEGVONADD(ior);
	lock_.unlock();

	return 0; //fb;
}

/**
 * find the next free region of size greater than nblocks
 */
rrec_t* XARecoveryLog::next_free(size_t nblocks) {
	size_t sz = 0;

	if (!log_.is_open())
		return 0;

	for (rrec_t* p = arena_; p; ) {
		if (p->magic == INUSE) {
			sz = 0;
			p = next_rec(p);
		} else if (++sz >= nblocks) {
			return p - sz + 1;
		} else {
			p += 1;
		}
	}

	return 0;
}

/**
 * there is a window during delete where a log record
 * can end up pointing to a free block. If that happened
 * it must have been the last action so provided check_log
 * is called when XARecoveryLog is constructed all will
 * be well.
 */
void XARecoveryLog::check_log() {
	for (rrec_t* rr = arena_; ;) {
		rrec_t* next = next_rec(rr);

		if (next == 0)
			return;

		if (rr->magic == INUSE && next->magic != INUSE) {
			// must have previously failed whilst deleting next
			LOG4CXX_INFO(xarcllogger, (char *) "fixing up recovery log");
			rr->next = next->next;
			sync_rec(&(rr->next), sizeof (rr->next));
		}

		rr = next;
	}
}

void XARecoveryLog::debug_dump(rrec_t* p, rrec_t* end) {
	LOG4CXX_TRACE(xarcllogger, (char *) "dumping from " << p << " to " << end);
//	LOG4CXX_TRACE(xarcllogger, (char *) "dumping arena: next=0x%x magic=0x%x formatID=0x%x recsz=0x%x ior=%s\n",
//		p->next, p->magic, p->xid.formatID, sizeof (rrec_t), IOR(p));

	while (p && p < end) {
		if (p->magic == INUSE) {
			LOG4CXX_TRACE(xarcllogger, (char *) "addr: " << p << " ior: " << IOR(p));
			p = next_rec(p);
		} else {
			p++;
		}
	}
}
