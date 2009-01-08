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

#ifndef org__xots__tx__Transaction_Impl_h
#define org__xots__tx__Transaction_Impl_h

#include "atmiBrokerTxMacro.h"

#include <cctype>
#ifdef TAO_COMP
#include "AtmiBrokerC.h"
#include "tao/PortableServer/PortableServerC.h"
#elif ORBIX_COMP
#include "AtmiBroker.hh"
#include <omg/PortableServer.hh>
#elif VBC_COMP
#include "AtmiBroker_s.hh"
#include <PortableServerExt_c.hh>
#endif

using namespace CosTransactions;

namespace org {
namespace xots {

namespace tx {

class ATMIBROKER_TX_DLL Transaction_Impl {
	friend class txFactory; // for constructor
	friend class Current_Impl; // need access to control()

	Transaction_Impl();

	CORBA::ORB_var m_orb;

	CORBA::ORB_ptr tx_orb() {
		return m_orb.in();
	}
	;

	PortableServer::POA_var m_txpoa;

	CosTransactions::Control_var m_control;
	CosTransactions::Terminator_var m_terminator;
	CosTransactions::Coordinator_var m_coordinator;

	CosTransactions::Coordinator_var m_parent_coordinator;

	CosTransactions::Coordinator_ptr parent_coordinator() const;

	CosTransactions::Coordinator_var m_top_level_coordinator;

	CosTransactions::Coordinator_ptr top_level_coordinator() const;

	void set_top_level_coordinator(CosTransactions::Coordinator_ptr top_coord);

	CosTransactions::Status m_status;

	CORBA::ULong m_timeout;

protected:

	void set_timeout(CORBA::ULong timeout) throw (CORBA::BAD_PARAM );

	CORBA::ULong get_timeout(void) const;

	string m_txname;

	CosTransactions::PropagationContext_var m_propctx;

	/** CTOR:
	 @param1: the transaction id of this transaction
	 @param2: a list of all tid of the ancestors
	 @param3: a ref. to the factory for the recovery coordinators
	 @param4: a pointer to the parent
	 @param5: top-level transaction (CORBA - pointers )
	 @param6: timeout
	 */
	Transaction_Impl(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa,
	//	    const ID&,
			//	    const ID_List ancestor_tid_list,
			//	    org::xots::recovery::RecoveryCoordinator_Factory* rcfac,
			CosTransactions::Coordinator* parent_coord, CosTransactions::Coordinator* top_level_coord, CORBA::ULong timeout);

	Transaction_Impl(const Transaction_Impl&);

public:

	void implements_control(CosTransactions::Control_ptr);
	void implements_terminator(CosTransactions::Terminator_ptr);
	void implements_coordinator(CosTransactions::Coordinator_ptr);

	CosTransactions::Coordinator_ptr coordinator() const;
	CosTransactions::Control_ptr control() const;
	//@}

	virtual ~Transaction_Impl();

public:
	///@name Implemenation of Control Interface
	//@{
	virtual CosTransactions::Terminator_ptr get_terminator() throw (CORBA::SystemException, CosTransactions::Unavailable );

	virtual CosTransactions::Coordinator_ptr get_coordinator() throw (CORBA::SystemException, CosTransactions::Unavailable );
	//@}


	///@name Implemenatation of Terminator Interface
	//@{
	virtual void commit(CORBA::Boolean report_heuristics) throw (CORBA::SystemException, CosTransactions::HeuristicMixed, CosTransactions::HeuristicHazard );

	virtual void rollback() throw (CORBA::SystemException );
	//@}


	///@name Implementation of Coordinator Interface, tx-status related
	//@{
	/// return the status of 'this' transaction, note exceptions
	virtual CosTransactions::Status get_status() const throw ();

	/// returns the status of the parent transaction, in case of top-tx: own status
	virtual CosTransactions::Status get_parent_status() const throw ();

	/// returns the status of the top level transaction
	virtual CosTransactions::Status get_top_level_status() const throw ();
	//@}


	///@name Implementation of Coordinator Interface, tx-tree related
	//@{
	/// returns true if 'this' and 'tc' represent the same transaction
	virtual CORBA::Boolean is_same_transaction(CosTransactions::Coordinator_ptr tc) const throw ();

	/// returns true if 'this' and the parameter have at least one ancestor in common
	virtual CORBA::Boolean is_related_transaction(CosTransactions::Coordinator_ptr tc) const throw ();

	/// returns true if 'this' is an ancestor of the tx represented by the parameter
	virtual CORBA::Boolean is_ancestor_transaction(CosTransactions::Coordinator_ptr tc) const throw ();

	/// returns true if 'this' is an descendant of the tx represented by the parameter
	virtual CORBA::Boolean is_descendant_transaction(CosTransactions::Coordinator_ptr tc) const throw ();

	/// returns true if this represents a top level transaction
	virtual CORBA::Boolean is_top_level_transaction() const throw ();
	//@}


	///@name Implementation of Coordinator Interface, tx-identification related
	//@{
	/// returns a hash value for this transaction
	virtual CORBA::ULong hash_transaction() const throw ();

	/// returns a hash value of the top-level transaction
	virtual CORBA::ULong hash_top_level_tran() const throw ();

	/// returns the name of this transaction
	virtual char* get_transaction_name() const throw();
	//@}


	///@name Implementation of Coordinator Interface, 2PC participants registration related
	//@{
	/** registers a resource and returns the responsible recovery coordinator.<P>
	 The resource will participate in the 2PC protocoll.
	 */
	virtual CosTransactions::RecoveryCoordinator_ptr register_resource(CosTransactions::Resource_ptr rr) throw (CORBA::SystemException, CosTransactions::Inactive );

	/** figures out the synchronator and register the sync object with it.
	 */
	virtual void register_synchronization(CosTransactions::Synchronization_ptr sync) throw (CORBA::SystemException, CosTransactions::Inactive, CosTransactions::SynchronizationUnavailable );

	/** registers a ressource, which is aware of subtransactions and will be informed
	 when a subransaction commits or aborts.
	 */
	virtual void register_subtran_aware(CosTransactions::SubtransactionAwareResource_ptr rr) throw (CORBA::SystemException, CosTransactions::Inactive, CosTransactions::NotSubtransaction );
	//@}


	///@name Implementation of Coordinator Interface, transaction start and termination
	//@{
	/// creates a subtransaction and returns the associated Control object
	virtual CosTransactions::Control_ptr create_subtransaction() throw (CORBA::SystemException, CosTransactions::SubtransactionsUnavailable, CosTransactions::Inactive );
	//@}


	///@name Implementation of Coordinator Interface, others
	//@{
	/// ensures that abort is the only possible outcome for this transaction
	virtual void rollback_only() throw (CORBA::SystemException, CosTransactions::Inactive );

	/// for inter-orb portability and inter-orb context propagation: NOT IMPLEMENTED
	virtual CosTransactions::PropagationContext * get_txcontext() throw (CORBA::SystemException, CosTransactions::Unavailable );
	//@}

	/*
	 ============================================================================
	 CORBA Interfaces END HERE
	 ============================================================================
	 */
};

}
}
}

/**
 * @file org/xots/tx/Transaction_Impl.h
 * $Header: /cvsroot/xots/xots/xots/src/org/xots/tx/Transaction_Impl.h,v 1.9 2003/07/23 10:06:29 chris-liebig Exp $
 * symbolic $Name:  $
 *
 *   $Log: Transaction_Impl.h,v $
 *   Revision 1.9  2003/07/23 10:06:29  chris-liebig
 *   savepoint before TUD checkout for solaris built
 *
 *   Revision 1.8  2003/06/14 22:12:02  chris-liebig
 *   gcc 3.0 compile ok
 *
 *   Revision 1.7  2003/06/01 21:54:03  chris-liebig
 *   bulk checkin before release tests
 *
 *   Revision 1.6  2003/01/12 19:52:35  chris-liebig
 *   added timeout support
 *   see simple test lines in test/RCsvc/main.cpp
 *
 *   Revision 1.5  2003/01/03 20:12:29  chris-liebig
 *   doxygen comment fiexd
 *
 *   Revision 1.4  2002/11/03 18:42:21  chris-liebig
 *   use of guarded mutex for termination critical section
 *
 *   Revision 1.3  2002/08/05 20:38:43  chris-liebig
 *   reworked Propagator to support propagation of top_level_coordinator reference
 *   this is need for CTX_REG_IMMEDIATE policy, which is also new (but the standard OTS policy !)
 *   by setting "xots.context.registration immediate", you will have default interposed registration
 *   behaviour, i.e. right after intercepting the incoming call before doing any harm to
 *   the resource.
 *   our optimization is "xots.context.registration deferred", which piggy backs the interposed's
 *   reousrce in the reply. this carries the possiility of inconsitencies in some failure case !
 *
 *   Revision 1.2  2002/07/14 18:16:21  chris-liebig
 *   adopted to new tokenizer interfaces
 *
 *   Revision 1.1.1.1  2002/06/15 19:35:01  chris-liebig
 *   pre_alpha
 *
 *   Revision 1.1.1.1  2002/06/15 08:51:32  chris
 *   upload to sourceorge
 *
 *
 */
#endif

