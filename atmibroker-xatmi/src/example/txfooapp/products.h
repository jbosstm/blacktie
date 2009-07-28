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
#ifndef __PRODUCTS_H
#define __PRODUCTS_H

#include "request.h"

/**
 * This file defines the 
 */
extern int null_access(test_req_t *req, test_req_t *resp);
extern int ora_access(test_req_t *req, test_req_t *resp);
extern int bdb_access(test_req_t *req, test_req_t *resp);

extern long null_xaflags();
extern long ora_xaflags();
extern long bdb_xaflags();

/*
 * some RMs do not allow mixed access - in fact Berkeley Db doesn't even support
 * 2 dbs in different files one accessed remotely and the other locally
 */
#define	REMOTE_ACCESS	0x1
#define	LOCAL_ACCESS	0x2
#define	ANY_ACCESS	(REMOTE_ACCESS & LOCAL_ACCESS)

typedef struct product {
    int id;	/* id for the product (used by servers to index into products) */
	const char *pname;	/* a name for the product configuration */
	const char *dbname;	/* identifies the database */
	int loc;	/* bitmap indicating what kind of access is allowed */
    int (*access)(test_req_t *, test_req_t *);	// function for doing CRUD operations
	long (*xaflags)();	// function to return the flags supported by the Resource Manager
} product_t;

product_t products[] = {
	{0, "null db", "null", ANY_ACCESS, null_access, null_xaflags},
	{1, "ora - blacktie", "blacktie", ANY_ACCESS, ora_access, ora_xaflags},
	{2, "bdb - db1", "db1", REMOTE_ACCESS, bdb_access, bdb_xaflags},
	{3, "bdb - db2", "db2", REMOTE_ACCESS, bdb_access, bdb_xaflags},
	{-1, 0, 0, 0, 0},
};

#endif /* __PRODUCTS_H */
