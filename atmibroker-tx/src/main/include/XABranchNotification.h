#ifndef _XABRANCHNOTIFICATION_H
#define _XABRANCHNOTIFICATION_H

#include "xa.h"

class XABranchNotification {
public:
	virtual ~XABranchNotification() {};

	virtual void set_complete(XID * xid) = 0;
	virtual void notify_error(XID *, int, bool) = 0;
};
#endif // _XABRANCHNOTIFICATION_H
