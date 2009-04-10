/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
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
#ifndef OrbManagement_H
#define OrbManagement_H

#include "atmiBrokerCoreMacro.h"

#include "CorbaConnection.h"

extern BLACKTIE_CORE_DLL CORBA_CONNECTION* initOrb(char* name);
extern BLACKTIE_CORE_DLL void shutdownBindings(CORBA_CONNECTION* connection);

/**
 * locate an orb by name
 * - input parameter is the orb id of the target orb. If it is NULL then an
 *   arbitary orb will be returned
 */
extern BLACKTIE_CORE_DLL CORBA::ORB_ptr find_orb(const char *);

/**
 * convert a object into an IOR:
 * the first parameter is ptr to a Corba object
 * the second parameter is the name of the orb that owns t (if NULL then
 * an arbitary orb will be used to perform the conversion)he object
 */
extern BLACKTIE_CORE_DLL char* atmi_object_to_string(CORBA::Object_ptr, char *);

/**
 * convert an IOR into a Corba object:
 * the input parameter is the name of the orb that owns the IOR (if NULL then
 * an arbitary orb will be used to perform the conversion)
 */
extern BLACKTIE_CORE_DLL CORBA::Object_ptr atmi_string_to_object(char *, char *);

#endif
