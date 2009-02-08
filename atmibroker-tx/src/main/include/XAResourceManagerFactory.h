/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat Middleware LLC, and others contributors as indicated
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
#ifndef XARESOURCEADAPTORFACTORY_H
#define XARESOURCEADAPTORFACTORY_H

#include "XAResourceManager.h"

#include <map>

typedef std::map<const char *, XAResourceManager *> ResourceManagerMap;

/*
 * XAResourceManagerFactory
 *
 * XAResourceManager
 *
 * XAResourceAdaptorImpl
 */
class ATMIBROKER_TX_DLL XAResourceManagerFactory
{
public:
	XAResourceManagerFactory();
	~XAResourceManagerFactory();

	XAResourceManager * findRM(const char *);
	void createRMs(CONNECTION *) throw (RMException);
	void destroyRMs(CONNECTION *);
	void startRMs(CONNECTION *);
	void endRMs(CONNECTION *);
	void suspendRMs(CONNECTION *);
	void resumeRMs(CONNECTION *);

private:
        ResourceManagerMap rms_;

	XAResourceManager * createRM(
		CONNECTION *, long, const char *, const char *, const char *, const char *, const char *) throw (RMException);
};
#endif	// XARESOURCEADAPTORFACTORY_H
