/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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

#include "ace/TSS_T.h"
#include <map>
#include <iostream>

class TSSData {
public:
	~TSSData() {
	}
	bool set(int k, void* v) {
		tssmap[k] = v;
		return true;
	}
	void* get(int k) {
		return tssmap[k];
	}
	bool destroy(int k) {
		tssmap[k] = 0;
		return true;
	}

	std::map<int, void*> tssmap;
};

static ACE_TSS<TSSData> tss;

extern int getKey() {
	return -1;
}

extern bool setSpecific(int key, void* threadData) {
	return tss->set(key, threadData);
}

extern void* getSpecific(int key) {
	return tss->get(key);
}

extern bool destroySpecific(int key) {
	return tss->destroy(key);
}
