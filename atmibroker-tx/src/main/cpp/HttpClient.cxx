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
#include <stdlib.h>
#include <string.h>

#include "HttpClient.h"
#include "log4cxx/logger.h"

log4cxx::LoggerPtr httpclientlog(log4cxx::Logger::getLogger("TxHttpClient"));

char * HttpClient::send(struct mg_request_info* ri,
	const char* method, const char* uri, const char* mediaType, const char *body, size_t *rcnt) {
	size_t blen = (body == NULL ? 0 : strlen(body));
	char host[1025], buf[BUFSIZ];
	int port, is_ssl = 0, n;

	(void) parse_url(uri, host, &port);
	LOG4CXX_DEBUG(httpclientlog, "connected to TM on " << host << ":" << port << " URI=" << uri);

	struct mg_connection *conn = get_connection(host, port, is_ssl);

	if (conn == NULL) {
		//perror("could not connect");
		LOG4CXX_WARN(httpclientlog, "Unable to connect to TM on " << host << ":" << port);
		return NULL;
	}

	LOG4CXX_DEBUG(httpclientlog, "connected to TM on " << host << ":" << port << " " << method << " " << uri);
	mg_printf(conn, "%s %s HTTP/%s\r\n", method, uri, VERSION);
	mg_printf(conn, "Host: %s\r\n", host);
	mg_printf(conn, "%s: %d\r\n", "Content-Length", blen);
	mg_printf(conn, "%s: %s\r\n", "Content-Type", mediaType);

	// End of headers, final newline
	mg_write(conn, "\r\n", 2);

	// write any body data
	if (blen > 0)
		mg_write(conn, body, blen);

	// read the response
	//memset(buf, 0, sizeof(buf));
	LOG4CXX_DEBUG(httpclientlog, "wrote header: body len: " << blen << " content: " << body);
	int nread = 0;
	n = read_bytes(conn, buf, sizeof(buf), &nread);
	char *content = (nread > n ? mg_strndup(buf + n, nread - n) : NULL);
	char *b = & buf[0];
	char *scode;

	if (rcnt != NULL)
		*rcnt = (size_t) (nread - n + 1);

	LOG4CXX_DEBUG(httpclientlog, "read response: " << n << " and " << nread);

	// parse response
	// RFC says that all initial whitespaces should be ingored
	while (*b != '\0' && isspace(* (unsigned char *) b))
		b++;

	ri->http_version = skip(&b, " ");
	scode = skip(&b, "\r\n ");
	ri->status_code = (scode != NULL ? atoi(scode) : -1);
	skip(&b, "\r\n");

	LOG4CXX_DEBUG(httpclientlog, "http resonse " << scode << " as int: " << ri->status_code);

	if (strncmp(ri->http_version, "HTTP/", 5) == 0) {
		ri->http_version += 5;   /* Skip "HTTP/" */
		parse_http_headers(&b, ri);

		for (int i = 0; i < ri->num_headers; i++) {
			LOG4CXX_TRACE(httpclientlog, "Header:\t" << ri->http_headers[i].name <<
				"=" << ri->http_headers[i].value);
		}

		if (content != NULL) {
			const char *clen = get_header(ri, "Content-Length");

			if (clen != NULL)
				*(content + atoi(clen)) = '\0';
		}
	} else {
		ri->status_code = 500;
		LOG4CXX_WARN(httpclientlog, "cannot handle http version: " << ri->http_version);
	}

	// done with the connection
	LOG4CXX_DEBUG(httpclientlog, "closing connection: status=" << ri->status_code);
	close_connection(conn);
	free(conn);

	return content;
}
