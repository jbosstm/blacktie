#ifndef _XASTATEMODEL_H
#define _XASTATEMODEL_H

#include <string>
#include "xa.h"

namespace atmibroker {
	namespace xa {

/*
 * State table for managing legal calling sequences for XA routines.
 * Refer to chapter 6 of X/Open XA specification for details.
 */

enum XAEVENT {
	XACALL_START = 0,
	XACALL_END,
	XACALL_OPEN,
	XACALL_RECOVER,
	XACALL_CLOSE,
	XACALL_PREPARE = 5,
	XACALL_COMMIT,
	XACALL_ROLLBACK,
	XACALL_FORGET,
};

enum XAASTATE {
	T0 = 0x0,   // not associated
	T1 = 0x1,   // associated
	T2 = 0x2,   // association suspended
};

enum XABSTATE {
	S0 = 0x0,   // Non-existent transaction
	S1 = 0x1,   // active
	S2 = 0x2,   // idle
	S3 = 0x4,   // prepared
	S4 = 0x8,   // rollback only
	S5 = 0x10,  // heuristically completed

	R0 = 0x20,  // RM not initialised
//  R1 = 0x40,  // RM initialised
};

class BLACKTIE_TX_DLL XAStateModel
{
public:
	XAStateModel();

	int transition(XID& xid, enum XAEVENT method, long flags, int rv);

	int astate() {return astate_;} // thread association state
	int bstate() {return bstate_;} // branch state

	static std::string show_flags(long flags);

private:
	int transition(int allowable[], int *initial_state, int next_state);
	int btransition(int *ini_state, enum XAEVENT method, long flag, int rval);
	int atransition(int *ini_state, enum XAEVENT method, long flag, int rval);

	int astate_;
	int bstate_;
};

} //	namespace xa
} //namespace atmibroker

#endif // _XASTATEMODEL_H
