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
/*#include <userlogc.h>*/

#include <stdlib.h>
#include "ace/OS_NS_unistd.h"

static fault_t *faults = 0;

static fault_t *last_fault() {
	fault_t *last;

	for (last = faults; last; last = last->next) {
		if (!last->next)
			return last;
	}

	return 0;
}

int dummy_rm_del_fault(int id)
{
	fault_t *curr, *prev = 0;

	/*printf("dummy_rm: del_fault: %d", id);*/
	for (curr = faults; curr; prev = curr, curr = curr->next) {
		if (curr->id == id) {
			if (prev == NULL)
				faults = curr->next;
			else
				prev->next = curr->next;

			free(curr);

			return 0;
		}
	}

	return -1;
}

int dummy_rm_add_fault(fault_t *fault)
{
	fault_t *last;

	/*printf("dummy_rm: del_fault:");*/

	if (fault == 0)
		return 1;

	last = last_fault();

	fault->next = 0;

	if (last == 0) {
		faults = (fault_t *) malloc(sizeof(fault_t));
		fault->id = 0;
		*faults = *fault;
	} else {
		last->next = (fault_t *) malloc(sizeof(fault_t));
		fault->id = last->id + 1;
		*(last->next) = *fault;
	}

	return 0;
}
static int apply_faults(enum XA_OP op, int rmid)
{
	fault_t *f;

	/*printf("dummy_rm: apply_faults: op=%d rmid=%d", op, rmid);*/

	for (f = faults; f; f = f->next) {
		if (f->rmid == rmid && f->op == op) {
			/*printf("dummy_rm: applying fault to op %d rc %d\n", op, f->rc);*/
			switch (f->xf) {
			default:
				break;
			case F_HALT:
				/* generate a SEGV fault */
				f->arg = 0;
				*((char *) f->arg) = 0;
				break;
			case F_DELAY:
				long* timeout = (long*)f->arg;
				(void) ACE_OS::sleep(*timeout);
				break;
			}

			return f->rc;
		}
	}

	return XA_OK;
}

static int open(char *a, int i, long l) {
	return apply_faults(O_XA_OPEN, i);
}
static int close(char *a, int i, long l) {
	return apply_faults(O_XA_CLOSE, i);
}

static int start(XID *x, int i, long l) {
	return apply_faults(O_XA_START, i);
}
static int end(XID *x, int i, long l) {
	return apply_faults(O_XA_END, i);
}
static int prepare(XID *x, int i, long l) {
	return apply_faults(O_XA_PREPARE, i);
}
static int commit(XID *x, int i, long l) {
	return apply_faults(O_XA_COMMIT, i);
}
static int rollback(XID *x, int i, long l) {
	return apply_faults(O_XA_ROLLBACK, i);
}
static int recover(XID *xid, long l1, int i, long l2) {
	return apply_faults(O_XA_RECOVER, i);
}

static int forget(XID *x, int i, long l) {
	return apply_faults(O_XA_FORGET, i);
}

static int complete(int *ip1, int *ip2, int i, long l) {
	return apply_faults(O_XA_COMPLETE, i);
}

struct xa_switch_t testxasw = {
	"DummyRM", 0L, 0,
	open, close,
	start, end, rollback, prepare, commit,
	recover,
	forget,
	complete
};

