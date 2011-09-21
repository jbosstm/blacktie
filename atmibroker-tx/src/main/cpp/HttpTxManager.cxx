/*
 * JBoss, Home of Professional Open Source
 * Copyright 2011, Red Hat, Inc., and others contributors as indicated
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
#include "HttpTxManager.h"
#include "HttpControl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

namespace atmibroker {
	namespace tx {

static const char* HTTP_400 = (char *) "Bad Request";
static const char* HTTP_409 = (char *) "Conflict";
static const char* HTTP_412 = (char *) "Precondition Failed";

static const char XIDPAT[] = "/xid/";
static const char ENDPAT[] = "/terminate";

log4cxx::LoggerPtr httptxlogger(log4cxx::Logger::getLogger("TxHttpTxManager"));

static char * parse_wid(const char *s) {
	const char *b = strstr(s, XIDPAT);
	const char *e = strstr(s, ENDPAT);
	char *wid = 0;

	if (b && e && (b += sizeof (XIDPAT) - 1) < e) {
		char *buf = mg_strndup(b, e - b);

		if (buf) {
			wid = (char *) malloc(1024);
			url_encode(buf, wid, 1024);
			free(buf);
		}
	}

	return wid;
}

HttpTxManager::HttpTxManager(const char *txmUrl, const char *resUrl) :
	TxManager() {
	_txmUrl = mg_strdup(txmUrl);
	_resUrl = mg_strdup(resUrl);
	FTRACE(httptxlogger, "ENTER inst create " << this);
}

HttpTxManager::~HttpTxManager() {
	FTRACE(httptxlogger, "ENTER inst destroy " << this);
	if (_ws != NULL) {
		LOG4CXX_WARN(httptxlogger, "Close has not been called");
		do_close();
	}

	if (_txmUrl != NULL)
		free(_txmUrl);
	if (_resUrl != NULL)
		free(_resUrl);
}

TxManager* HttpTxManager::create(const char *txmUrl, const char *resUrl) {
	if (_instance == NULL)
		_instance = new HttpTxManager(txmUrl, resUrl);

	return _instance;
}

int HttpTxManager::associate_transaction(char* txn, long ttl) {
	FTRACE(httptxlogger, "ENTER" << txn);

	TxControl *tx = new HttpControl(txn, ttl, ACE_OS::thr_self());
	int rc = tx_resume(tx, TMJOIN);

	if (rc != XA_OK)
		delete tx;

	return rc;
}

// return the transaction URL
char *HttpTxManager::get_current(long* ttl) {
	return (char *) get_control(ttl);
}

void HttpTxManager::release_control(void *ctrl) {
	if (ctrl != NULL)
		free(ctrl);
}

TxControl* HttpTxManager::create_tx(long timeout) {
	HttpControl* tx = new HttpControl(timeout, ACE_OS::thr_self());

	if (tx->start(_txmUrl) != TX_OK) {
		LOG4CXX_INFO(httptxlogger, "Unable to start a new txn on URI ");
		delete tx;
		return NULL;
	}

	return tx;
}

char *HttpTxManager::enlist(XAWrapper* resource, TxControl *tx, const char * xid) {
	if (guard(_isOpen) != TX_OK)
		return NULL;

	char *recUrl = tx->enlist(_ws->get_host(), _ws->get_port(), xid);

	if (recUrl != NULL)
		_branches[resource->get_name()] = resource;

	return recUrl;
}

int HttpTxManager::do_open(void) {
	char host[1025];
	int port;

	(void) parse_url(_resUrl, host, &port);

	FTRACE(httptxlogger, "ENTER: opening HTTP server: host: " <<
		host << " port: " << port << " handler: " << this);
	_ws = new HttpServer(host, port);

	_ws->add(this);

	return TX_OK;
}

int HttpTxManager::do_close(void) {
	if (_ws != NULL) {
		FTRACE(httptxlogger, "ENTER: closing HTTP server: handler " << this);
		_ws->remove(this);
		delete _ws;
		_ws = NULL;
	}

	return TX_OK;
}

XAWrapper * HttpTxManager::locate_branch(const char * xid)
{
	XABranchMap::iterator iter;

	for (iter = _branches.begin(); iter != _branches.end(); ++iter)
		if (strcmp(iter->first, xid) == 0)
			return (*iter).second;

	return NULL;
}

bool HttpTxManager::handle_request(
	struct mg_connection *conn, const struct mg_request_info *ri, const char *content, size_t len)
{
	int code = 400;
	const char* codestr = HTTP_400;
	const char *status = "";
	size_t plen = strlen(HttpControl::TXSTATUS);

	LOG4CXX_INFO(httptxlogger, "tm_request: method: " << ri->request_method << " uri: " << ri->uri <<
		" status_code: " << ri->status_code << " num_headers: " << ri->num_headers <<
		" qs: " << ri->query_string << " content: " << content);

	for (int i = 0; i < ri->num_headers; i++)
		LOG4CXX_DEBUG(httptxlogger, "\t" << ri->http_headers[i].name <<
			"=" << ri->http_headers[i].value);

	if (plen < len)
		content += plen;

	int stat = HttpControl::http_to_tx_status(content);

	if (stat != -1) {
		char* wid = parse_wid(ri->uri);
		LOG4CXX_DEBUG(httptxlogger, "looking up branch " << wid);
		XAWrapper* branch = locate_branch(wid);

		if (wid != NULL)
			free(wid);

		LOG4CXX_DEBUG(httptxlogger, "branch: 0x" << branch << " content: " << content);

		if (branch) {
			// ? TODO should this logic be moved into a subclass of XAWrapper
			// TODO check that all possible XA codes are being mapped correctly
			int res = XAER_INVAL;
			int how = 0;

			if (stat == HttpControl::PREPARED_STATUS) {
				res = branch->do_prepare();
				switch (res) {
				case XA_OK:
					code = 200;
					status = HttpControl::PREPARED;
					break;
				case XA_RDONLY:
					code = 200;
					status = HttpControl::READONLY;
					break;
				case XAER_PROTO:
					code = 412;
					codestr = HTTP_412;
					status = HttpControl::ABORTED;
					break;
				default:
					status = HttpControl::ABORTED;
					code = 409;
					codestr = HTTP_409;
					break;
				}
			} else if (stat == HttpControl::COMMITTED_STATUS) {
				res = branch->do_commit();
				how = 1;
				status = HttpControl::COMMITTED;
			} else if (stat == HttpControl::COMMITTED_ONE_PHASE_STATUS) {
				res = branch->do_commit_one_phase();
				how = 1;
				status = HttpControl::COMMITTED;
			} else if (stat == HttpControl::ABORTED_STATUS) {
				how = 2;
				res = branch->do_rollback();
				status = HttpControl::ABORTED;
			} else {
				LOG4CXX_DEBUG(httptxlogger, "invalid request: " << stat);
				code = 400;
				codestr = HTTP_400;
				status = "";
			}

			// If the operation fails, e.g., the participant cannot be prepared, then the implementation MUST
			// return 409. It is implementation dependant as to whether the participant-resource or related URIs
			// remain valid, i.e., an implementation MAY delete the resource as a result of a failure. Depending
			// upon the point in the two-phase commit protocol where such a failure occurs the transaction
			// MUST be rolled back. If the participant is not in the correct state for the requested operation,
			// eg Prepare when it has been already been prepared, then the implementation MUST return 412.
			// If the transaction coordinator receives any response other than 200 for Prepare then the
			// transaction MUST rollback.

			if (how != 0) {
				switch (res) {
				default:
					LOG4CXX_DEBUG(httptxlogger, "Return 200 OK with body: " <<
						status << " XA status: " << res);
					code = 200;
					break;
				case XAER_PROTO:
					// TODO Do all PROTO errors mean rollback for example (at least with ORACLE)
					// committing without preparing generates XAER_PROTO and rolls back the branch
					status = HttpControl::ABORTED;
					code = 412;
					codestr = HTTP_412;
					break;
				case XA_HEURHAZ:
					status = HttpControl::H_HAZARD;
					code = 409;
					codestr = HTTP_409;
					break;	
				case XA_HEURCOM:
					//status = HttpControl::H_COMMIT;
					status = (how == 0 ? HttpControl::COMMITTED : HttpControl::H_ROLLBACK);
					code = 409;
					codestr = HTTP_409;
					break;
				case XA_HEURRB:
				case XA_RBROLLBACK: // these codes may be returned only if the TMONEPHASE flag was set
				case XA_RBCOMMFAIL:
				case XA_RBDEADLOCK:
				case XA_RBINTEGRITY:
				case XA_RBOTHER:
				case XA_RBPROTO:
				case XA_RBTIMEOUT:
				case XA_RBTRANSIENT:
					//status = HttpControl::H_ROLLBACK;
					status = (how == 0 ? HttpControl::H_ROLLBACK : HttpControl::ABORTED);
					code = 409;
					codestr = HTTP_409;
					break;
				case XA_HEURMIX:
					status = HttpControl::H_MIXED;
					code = 409;
					codestr = HTTP_409;
					break;
				case XAER_INVAL:
					status = "";
					code = 400;
					codestr = HTTP_400;
					break;
				}

				LOG4CXX_DEBUG(httptxlogger, "completion request " << content <<
					" XA status: " << res << " RTS status: " << status << " http status: " << code);
			}
		}
	} else {
		LOG4CXX_DEBUG(httptxlogger, "either no content (" << content << ") or prefix (" <<
			HttpControl::TXSTATUS << ") does not match");
	}

	LOG4CXX_DEBUG(httptxlogger, "completion request returning " <<
		" RTS status: " << status << " http status: " << code);

	if (code == 200 || code == 409) {
		mg_printf(conn, "HTTP/1.1 %d %s\r\n"
			"Content-Length: %d\r\n"
			"Content-Type: application/txstatus\r\n\r\n"
			"%s%s",
			200, "OK", strlen(status), HttpControl::TXSTATUS, status);
	} else {
		mg_printf(conn, "HTTP/1.1 %d %s\r\n"
			"Content-Length: %d\r\n"
			"Content-Type: application/txstatus\r\n\r\n"
//			"Connection: %s\r\n\r\n"	// "keep-alive" or "close"
			"%s%s",
			code, codestr, strlen(status), HttpControl::TXSTATUS, status);
	}

	return true;
}

}
}
