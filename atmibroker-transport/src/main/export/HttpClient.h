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
#ifndef _HTTPCLIENT_H
#define _HTTPCLIENT_H

#include "httpTransportMacro.h"
#include "mongoose.h"

class BLACKTIE_HTTP_TRANSPORT_DLL HttpClient {
public:
	HttpClient() : HTTP_PROTO_VERSION("1.1") {};
	virtual ~HttpClient() {};

	int send(struct mg_request_info* ri,
		const char* method, const char* uri, const char* mediaType,
		const char* headers[], const char *body, size_t blen, char **resp, size_t* rcnt);
	void dispose(struct mg_request_info* ri);

	const char *get_header(const struct mg_request_info *ri, const char *name);
	int parse_url(const char *url, char *host, int *port);
	void url_encode(const char *src, char *dst, size_t dst_len);

	int write(struct mg_connection *conn, const void *buf, size_t len);
	void close_connection(struct mg_connection *conn);

	const char *HTTP_PROTO_VERSION;

private:
	void dup_headers(struct mg_request_info* ri);

};
#endif //_HTTPCLIENT_H
