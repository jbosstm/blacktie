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
/*
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT

#ifndef AtmiBroker_ENV_XML_H_
#define AtmiBroker_ENV_XML_H_

#include "atmiBrokerCoreMacro.h"

#include <string.h>

#ifdef TAO_COMP
#include "tao/ORB.h"
#include "AtmiBrokerC.h"
#elif ORBIX_COMP
#include <omg/orb.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#endif

//#include "AtmiBroker_ServerImpl.h"

class ATMIBROKER_CORE_DLL AtmiBrokerEnvXml {
public:

	static const char* Environment_Begin_Tag;
	static const char* Environment_End_Tag;

	static const char* Environment_Desc_Begin_Tag;
	static const char* Environment_Desc_End_Tag;

	static const char* Server_Begin_Tag;
	static const char* Server_End_Tag;

	static const char* Company_Begin_Tag;
	static const char* Company_End_Tag;

	static const char* Domain_Begin_Tag;
	static const char* Domain_End_Tag;

	static const char* QSpace_Name_Begin_Tag;
	static const char* QSpace_Name_End_Tag;

	static const char* Naming_Service_Id_Begin_Tag;
	static const char* Naming_Service_Id_End_Tag;

	static const char* Notify_Service_Id_Begin_Tag;
	static const char* Notify_Service_Id_End_Tag;

	static const char* Logging_Service_Id_Begin_Tag;
	static const char* Logging_Service_Id_End_Tag;

	static const char* Trans_Factory_Id_Begin_Tag;
	static const char* Trans_Factory_Id_End_Tag;

	static const char* XA_Resource_Mgr_Id_Begin_Tag;
	static const char* XA_Resource_Mgr_Id_End_Tag;

	static const char* XA_Resource_Name_Begin_Tag;
	static const char* XA_Resource_Name_End_Tag;

	static const char* XA_Open_String_Begin_Tag;
	static const char* XA_Open_String_End_Tag;

	static const char* XA_Close_String_Begin_Tag;
	static const char* XA_Close_String_End_Tag;

	static const char* XA_Thread_Model_Begin_Tag;
	static const char* XA_Thread_Model_End_Tag;

	static const char* XA_Automatic_Association_Begin_Tag;
	static const char* XA_Automatic_Association_End_Tag;

	static const char* XA_Dynamic_Registration_Optimization_Begin_Tag;
	static const char* XA_Dynamic_Registration_Optimization_End_Tag;

	static const char* Env_Variables_Begin_Tag;
	static const char* Env_Variables_End_Tag;

	static const char* Env_Variable_Begin_Tag;
	static const char* Env_Variable_End_Tag;

	static const char* Env_Name_Begin_Tag;
	static const char* Env_Name_End_Tag;

	static const char* Env_Value_Begin_Tag;
	static const char* Env_Value_End_Tag;

	AtmiBrokerEnvXml();

	~AtmiBrokerEnvXml();

	void writeXmlDescriptor(AtmiBroker::EnvVariableInfoSeq* serverData, const char * aDescriptorFileName);

	bool parseXmlDescriptor(AtmiBroker::EnvVariableInfoSeq*, const char * aDescriptorFileName);

};

extern ATMIBROKER_CORE_DLL char* company;
extern ATMIBROKER_CORE_DLL char domain[30];
extern ATMIBROKER_CORE_DLL char server[30];
extern ATMIBROKER_CORE_DLL char * loggingServiceId;
extern ATMIBROKER_CORE_DLL char * notifyServiceId;
extern ATMIBROKER_CORE_DLL char * namingServiceId;
extern ATMIBROKER_CORE_DLL char * queue_name;
extern ATMIBROKER_CORE_DLL char* transFactoryId;
extern ATMIBROKER_CORE_DLL char* xaResourceMgrId;
extern ATMIBROKER_CORE_DLL char* xaResourceName;
extern ATMIBROKER_CORE_DLL char* xaOpenString;
extern ATMIBROKER_CORE_DLL char* xaCloseString;
extern ATMIBROKER_CORE_DLL bool xaThreadModel;
extern ATMIBROKER_CORE_DLL bool xaAutomaticAssociation;
extern ATMIBROKER_CORE_DLL bool xaDynamicRegistrationOptimization;

#endif
