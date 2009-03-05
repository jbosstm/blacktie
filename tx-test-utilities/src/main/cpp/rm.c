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

static int fn1(char *a, int i, long l) { return 0; }
static int fn2(XID *x, int i, long l) { return 0; }
static int fn3(XID *xid, long l1, int i, long l2) { return 0; }
static int fn4(int *ip1, int *ip2, int i, long l) { return 0; }

struct xa_switch_t testxasw = { "DummyRM", 0L, 0, fn1, fn1, /* open and close */
        fn2, fn2, fn2, fn2, fn2, /*start, end, rollback, prepare, commit */
        fn3, /* recover */
        fn2, /* forget */
        fn4 /* complete */
};

