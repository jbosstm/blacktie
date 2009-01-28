#include "XAResourceAdaptorImpl.h"

// TODO this stuff needs to work/integrate with LocalResourceManagerCache::create_local_resource_manager
// TODO figure out the best design for it
XAResourceAdaptorImpl::XAResourceAdaptorImpl(CORBA::Long rid, struct xa_switch_t * xa_switch) :
	complete_(false), rid_(rid), xa_switch_(xa_switch) {
}
XAResourceAdaptorImpl::~XAResourceAdaptorImpl() {
}

// CosTransactions::Resource implementation
CosTransactions::Vote XAResourceAdaptorImpl::prepare() {
	return CosTransactions::VoteCommit;
}
void XAResourceAdaptorImpl::rollback() {
	complete_ = true;
}
void XAResourceAdaptorImpl::commit() {
	complete_ = true;
}
void XAResourceAdaptorImpl::forget() {
}
void XAResourceAdaptorImpl::commit_one_phase() {
	complete_ = true;
}

// accessors
bool XAResourceAdaptorImpl::is_complete() {
	return complete_;
}
CORBA::Long XAResourceAdaptorImpl::rid(void) {
	return this->rid_;
}

// XA methods
char* XAResourceAdaptorImpl::get_name() {
	return (char *) xa_switch_->name;
}
long XAResourceAdaptorImpl::get_flags() {
	return xa_switch_->flags;
}
long XAResourceAdaptorImpl::get_version() {
	return xa_switch_->version;
}
int XAResourceAdaptorImpl::xa_open (char * name, int rmid, long flags) {
	return xa_switch_->xa_open_entry(name, rmid, flags);
}
int XAResourceAdaptorImpl::xa_close (char * name, int rmid, long flags) {
	return xa_switch_->xa_close_entry(name, rmid, flags);
}
int XAResourceAdaptorImpl::xa_start (XID * txid, int rmid, long flags) {
	return xa_switch_->xa_start_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_end (XID * txid, int rmid, long flags) {
	return xa_switch_->xa_end_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_rollback (XID * txid, int rmid, long flags) {
	return xa_switch_->xa_rollback_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_prepare (XID * txid, int rmid, long flags) {
	return xa_switch_->xa_prepare_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_commit (XID * txid, int rmid, long flags) {
	return xa_switch_->xa_commit_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_recover (XID * txid, long xxx, int rmid, long flags) {
	return xa_switch_->xa_recover_entry(txid, xxx, rmid, flags);
}
int XAResourceAdaptorImpl::xa_forget (XID * txid, int rmid, long flags) {
	return xa_switch_->xa_forget_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_complete (int * handle, int * retvalue, int rmid, long flags) {
	return xa_switch_->xa_complete_entry(handle, retvalue, rmid, flags);
}
