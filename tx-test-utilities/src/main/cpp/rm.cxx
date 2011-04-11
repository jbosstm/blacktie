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
#include "xa.h"
#include "testrm.h"
#include "btlogger.h"
#include "SynchronizableObject.h"

#include <stdlib.h>
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_time.h"
#include "ace/OS_NS_sys_time.h"

static SynchronizableObject _lock;
static long counter = 0;
static fault_t *faults = 0;

struct xid_array {
	int count;
	int cursor;
	XID xids[10];
};

static XID gen_xid(long id, long sid, XID &gid)
{
	XID xid = {gid.formatID, gid.gtrid_length};
	int i;

	for (i = 0; i < gid.gtrid_length; i++)
		xid.data[i] = gid.data[i];

	ACE_Time_Value now = ACE_OS::gettimeofday();
	// the first long in the XID data must contain the RM id
	(void) sprintf(xid.data + i, "%ld:%ld:%ld:%ld:%ld", id, sid, ++counter, now.sec(), now.usec());
	xid.bqual_length = strlen(xid.data + i);

	return xid;
}

// caller should be holding _lock
static fault_t *last_fault() {
	fault_t *last;

	for (last = faults; last; last = last->next) {
		if (!last->next)
			return last;
	}

	return 0;
}

int dummy_rm_del_faults()
{
	fault_t *curr, *prev = 0;
	int rv = 0;

	btlogger_debug("dummy_rm: del_faults:");
	_lock.lock();
	for (curr = faults; curr; prev = curr, curr = curr->next) {
		if (prev == NULL)
			faults = curr->next;
		else
			prev->next = curr->next;

		if (curr->rmstate != NULL)
			free(curr->rmstate);

		free(curr);
		rv += 1;
	}
	_lock.unlock();

	btlogger_debug("dummy_rm: deleted %d faults", rv);

	return rv;
}

int dummy_rm_del_fault(int id)
{
	fault_t *curr, *prev = 0;

	btlogger_debug("dummy_rm: del_fault: %d", id);

	if (id == -1)
		return dummy_rm_del_faults();

	_lock.lock();
	for (curr = faults; curr; prev = curr, curr = curr->next) {
		if (curr->id == id) {
			if (prev == NULL)
				faults = curr->next;
			else
				prev->next = curr->next;

			if (curr->rmstate != NULL)
				free(curr->rmstate);

			free(curr);
			_lock.unlock();

			return 0;
		}
	}
	_lock.unlock();

	return -1;
}

int dummy_rm_add_fault(fault_t *fault)
{
	fault_t *last;

	btlogger_debug("dummy_rm: del_fault:");

	if (fault == 0)
		return 1;

	_lock.lock();
	last = last_fault();

	fault->next = 0;
	fault->orig = fault;
	fault->orig->res = 0;
	fault->orig->res2 = 0;

	if (fault->xf == F_ADD_XIDS) {
		int i;
		long larg = *((long*) fault->arg);
		struct xid_array *xids;
		XID gid = {1L, 1L, 0L};

		if (larg >= 10 || larg <= 0) {
			_lock.unlock();
			return 1;
		}

		xids = (struct xid_array *) calloc(1, sizeof(struct xid_array));
		fault->rmstate = xids;
		xids->cursor = 0;
		xids->count = larg;

		for (i = 0; i < xids->count; i++) {
			XID xid = gen_xid(fault->rmid, 0L, gid);
			memcpy(&(xids->xids[i]), &xid, sizeof (XID));
		}
	}

	if (last == 0) {
		faults = (fault_t *) calloc(1, sizeof(fault_t));
		fault->id = 0;
		*faults = *fault;	// alternatively use the callers fault_t
	} else {
		last->next = (fault_t *) calloc(1, sizeof(fault_t));
		fault->id = last->id + 1;
		*(last->next) = *fault;	// alternatively use the callers fault_t
	}
	_lock.unlock();

	return 0;
}

static int apply_faults(XID *xid, enum XA_OP op, int rmid)
{
	fault_t *f;
	long *larg;
	long fc = 0L;
	int rc = 0;

	btlogger_debug("dummy_rm: apply_faults: op=%d rmid=%d\n", op, rmid);

	_lock.lock();
	for (f = faults; f; f = f->next) {
		fc += 1;

		if (fc == 100)
			btlogger_debug("dummy_rm: too many fault specifications\n");
		if (f->rmid == rmid && f->op == op) {
			btlogger_debug("dummy_rm: applying fault %d to op %d rc %d\n", f->xf, op, f->rc);
			switch (f->xf) {
			default:
				break;
			case F_HALT:
				/* generate a SEGV fault */
				larg = 0;
				*larg = 0;
				break;
			case F_DELAY:
				larg = (long*) f->arg;

				if (larg == 0 || *larg < 0L || *larg > 3600L) {
					btlogger_warn("dummy_rm: sleep period is invalid arg=%ld", larg == 0 ? 0 : *larg);
				} else {
					btlogger_debug("dummy_rm: delaying for %ld seconds\n", *larg);
					(void) ACE_OS::sleep(*larg);
				}
				break;
			}

			rc = f->rc;
			_lock.unlock();
			btlogger_debug("dummy_rm: fault return: %d", rc);

			return rc;
		}
	}
	_lock.unlock();

	btlogger_debug("dummy_rm: fault return: XA_OK\n");
	return XA_OK;
}

static int open(char *a, int rmid, long l) {
	return apply_faults(NULL, O_XA_OPEN, rmid);
}
static int close(char *a, int rmid, long l) {
	return apply_faults(NULL, O_XA_CLOSE, rmid);
}

static int start(XID *x, int rmid, long l) {
	return apply_faults(x, O_XA_START, rmid);
}
static int end(XID *x, int rmid, long l) {
	return apply_faults(x, O_XA_END, rmid);
}
static int prepare(XID *x, int rmid, long l) {
	return apply_faults(x, O_XA_PREPARE, rmid);
}

// remove xid from any fault specifications used to support recovery testing
static void end_check(XID *xid, int rmid) {
	fault_t *f;

	// TODO move this into apply_faults (ie generalise apply_faults)
	_lock.lock();
	for (f = faults; f; f = f->next) {
		if (f->rmid == rmid && f->op == O_XA_RECOVER && f->xf == F_ADD_XIDS) {
			struct xid_array *xids = (struct xid_array *) f->rmstate;
			int i;

			for (i = 0; i < xids->count; i++) {
				if (memcmp(xid, &(xids->xids[i]), sizeof (XID)) == 0) {	// bytewise compare
					for (i = i + 1; i < xids->count; i++)
						memcpy(&(xids->xids[i - 1]), &(xids->xids[i]), sizeof (XID));

					xids->count -= 1;
//					f->orig->res = xids->count;
					f->orig->res += 1;
				}
			}
		}
	}
	_lock.unlock();
}

static int commit(XID *x, int rmid, long l) {
	int rv = apply_faults(x, O_XA_COMMIT, rmid);
	end_check(x, rmid);
	return rv;
}
static int rollback(XID *x, int rmid, long l) {
	int rv = apply_faults(x, O_XA_ROLLBACK, rmid);
	end_check(x, rmid);
	return rv;
}

static int recover(XID *xid, long l1, int rmid, long flags) {
	fault_t *f;
	int rv = apply_faults(NULL, O_XA_RECOVER, rmid);

	_lock.lock();
	for (f = faults; f; f = f->next) {
		if (f->rmid == rmid && f->op == O_XA_RECOVER && f->xf == F_ADD_XIDS) {
			struct xid_array *xids = (struct xid_array *) f->rmstate;
			int i;

			// note a recovery scan than spans multiple calls must be done in the same thread
			// - we don't check for this since this is only a dummy RM for testing particular
			// behaviour. Likewise we don't validate the TMNOFLAGS flag
			if ((xids == NULL && l1 > 0) || l1 < 0) {
				rv = XAER_INVAL;
				break;
			}

			if (flags & TMSTARTRSCAN)
				xids->cursor = 0;

			for (i = 0; i < l1 && xids->cursor <= xids->count; xids->cursor++, i++)
				xid[i] = xids->xids[xids->cursor];

			if (flags & TMENDRSCAN)
				xids->cursor = 0;

			// record the number of recovered XIDs in res and the number needing recovery in res2
			f->orig->res = 0;
			f->orig->res2 = xids->count;

			rv = (i > 0 ? i - 1 : i);
			break;
		}
	}
	_lock.unlock();

	return rv;
}

static int forget(XID *x, int rmid, long l) {
	int rv =  apply_faults(x, O_XA_FORGET, rmid);
	end_check(x, rmid);
	return rv;
}

static int complete(int *ip1, int *ip2, int rmid, long l) {
	return apply_faults(NULL, O_XA_COMPLETE, rmid);
}

struct xa_switch_t testxasw = {
	"DummyRM", 0L, 0,
	open, close,
	start, end, rollback, prepare, commit,
	recover,
	forget,
	complete
};

