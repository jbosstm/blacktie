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
#include <stdio.h>
//#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "HttpServer.h"
#include "log4cxx/logger.h"

log4cxx::LoggerPtr httpserverlog(log4cxx::Logger::getLogger("TxHttpServer"));

static void *callback(enum mg_event event,
					  struct mg_connection *conn,
					  const struct mg_request_info *ri) {

	if (ri->user_data == NULL) {
		LOG4CXX_INFO(httpserverlog, "ignoring HTTP request - no handler. Content-Length: " <<
			mg_get_header(conn, "Content-Length"));
		//printf("ignoring HTTP request, user data is not set\n");
		return (void *) "";  // Mark as processed
	}

	LOG4CXX_TRACE(httpserverlog, "HTTP callback: casting handler " << ri->user_data);
	HttpRequestHandler* handler = (HttpRequestHandler *) ri->user_data;
	LOG4CXX_TRACE(httpserverlog, "HTTP callback: handler: cast ok");

	if (event == MG_NEW_REQUEST) {
		const char* clen = mg_get_header(conn, "Content-Length");
		char *buf = 0;
		size_t len = 0;

		LOG4CXX_DEBUG(httpserverlog, "HTTP callback event Content-Length: " << clen);

		if (clen) {
			len = atoi(clen);
			buf = (char *) malloc(len + 1);
			(void) mg_read(conn, buf, len);
			buf[len] = '\0';
			LOG4CXX_DEBUG(httpserverlog, "request content: " << buf << " length: " << len);
		}

		handler->handle_request(conn, ri, buf, len);
		LOG4CXX_DEBUG(httpserverlog, "HTTP callback event handled");

		if (buf)
			free(buf);

		return (void *) "";  // Mark as processed
	} else {
		LOG4CXX_DEBUG(httpserverlog, "ignoring HTTP callback event " << event);
		printf("ignoring HTTP callback event %d\n", event);
		return NULL;
	}

}

HttpServer::HttpServer(const char* host, int port) :
	SynchronizableObject(), _handler(NULL), _port(port), _ctx(NULL)
{
	_host = mg_strdup(host);
}

HttpServer::~HttpServer() {
	LOG4CXX_DEBUG(httpserverlog, "Destructor: mongoose ctx: " << _ctx);
	if (_handler != NULL) {
		LOG4CXX_WARN(httpserverlog, "Destructor: mongoose ctx still has handlers: " << _handler);
		remove(_handler);
	}

	if (_host)
		free(_host);
}

bool HttpServer::add(HttpRequestHandler* handler, const char *uri_pattern) {
	char port[16];
	sprintf(port, "%d", _port);
	const char *options[] = {"listening_ports", port, NULL};

	remove(_handler);
	_handler = handler;
	lock();	// TODO test the return value
	_ctx = mg_start(&callback, _handler, options);
	unlock();
	LOG4CXX_INFO(httpserverlog, "mongoose running on port " << port << " with handler " << handler);

	return (_ctx == NULL ? false : true);
}

void HttpServer::remove(HttpRequestHandler* handler, const char *uri_pattern) {
	LOG4CXX_INFO(httpserverlog, "removing " << handler << " context: " << _ctx);
	lock();	// TODO test the return value
	if (handler != NULL)
		handler->unregistering();

	if (_ctx != NULL) {
		LOG4CXX_INFO(httpserverlog, "Stopping mongoose");
		mg_stop(_ctx);
	}
	unlock();

	_handler = NULL;
	_ctx = NULL;
}
