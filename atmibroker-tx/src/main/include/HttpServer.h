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
#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H

#include "HttpRequestHandler.h"
#include "SynchronizableObject.h"

class HttpServer : protected SynchronizableObject {
public:
	HttpServer(const char *host, int port);
	virtual ~HttpServer();

	bool add(HttpRequestHandler* handler, const char *uri_pattern = NULL);
	void remove(HttpRequestHandler* handler, const char *uri_pattern = NULL);

	int get_port() {return _port;};
	const char *get_host() {return _host;};

private:
	HttpRequestHandler *_handler;
	int _port;
	struct mg_context *_ctx;
	char *_host;
};
#endif // _HTTP_SERVER_H
