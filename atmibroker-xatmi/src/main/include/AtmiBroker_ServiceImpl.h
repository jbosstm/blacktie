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

//-----------------------------------------------------------------------------
// Edit the idlgen.cfg to have your own copyright notice placed here.
//-----------------------------------------------------------------------------

// Class: AtmiBroker_ServiceImpl
// A POA servant which implements of the AtmiBroker::Service interface
//

#ifndef ATMIBROKER_SERVICEIMPL_H_
#define ATMIBROKER_SERVICEIMPL_H_

#include "xatmi.h"
#include "Session.h"
#include "Message.h"
#include "MessageListener.h"
#include "Sender.h"
#include "Receiver.h"
#include "log4cxx/logger.h"


class AtmiBroker_ServiceImpl: public virtual Session, public virtual MessageListener {
public:
	AtmiBroker_ServiceImpl(char *serviceName, void(*func)(TPSVCINFO *));
	virtual ~AtmiBroker_ServiceImpl();

	void onMessage(MESSAGE message);

	void getId(int& id);
	void setSendTo(char* replyTo);
	Receiver * getReceiver();
	Sender * getSender();
protected:
	Sender* queueSender;
	Receiver* queueReceiver;
private:
	static log4cxx::LoggerPtr logger;
	char* m_serviceName;
	void (*m_func)(TPSVCINFO *);
};

#endif
