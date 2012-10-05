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
#include <ctype.h>
#include <errno.h>

#include "ace/ACE.h"

#include "HttpClient.h"

void HttpClient::dup_headers(struct mg_request_info* ri) {
    for (int i = 0; i < ri->num_headers; i++) {
        ri->http_headers[i].name = strdup(ri->http_headers[i].name);
        ri->http_headers[i].value = strdup(ri->http_headers[i].value);
    }
}

void HttpClient::dispose(struct mg_request_info* ri) {
#if 0
// no longer calling HttpClient::dup_headers
    for (int i = 0; i < ri->num_headers; i++) {
        free(ri->http_headers[i].name);
        free(ri->http_headers[i].value);
    }
#endif
	ri->num_headers = 0;
}


int HttpClient::send(struct mg_request_info* ri, const char* method, const char* uri,
	const char* mediaType, const char* headers[], const char *body, size_t blen, char **resp, size_t *rcnt) {
//	size_t blen = (body == NULL ? 0 : strlen(body));
	char host[1025], buf[BUFSIZ];
	int port = 0, is_ssl = 0, n;

	ri->num_headers = 0;
	(void) parse_url(uri, host, &port);
//	LOG4CXX_DEBUG(httpclientlog, "connected to TM on " << host << ":" << port << " URI=" << uri);

	struct mg_connection *conn = get_connection(host, port, is_ssl);

	if (conn == NULL) {
		//perror("could not connect");
//		LOG4CXX_WARN(httpclientlog, "Unable to connect to TM on " << host << ":" << port);
		if (resp)
			*resp = NULL;

		ri->status_code = -1;
		return errno;
	}

//	LOG4CXX_DEBUG(httpclientlog, "connected to TM on " << host << ":" << port << " " << method << " " << uri);
	mg_printf(conn, "%s %s HTTP/%s\r\n", method, uri, HTTP_PROTO_VERSION);
	mg_printf(conn, "Host: %s\r\n", host);
	mg_printf(conn, "%s: %d\r\n", "Content-Length", blen);
	mg_printf(conn, "%s: %s\r\n", "Content-Type", mediaType);

    if (headers != NULL) {
        int i = 0;

        for (; headers[i]; i++) {
            mg_printf(conn, "%s\r\n", headers[i]);
        }
    }

	// End of headers, final newline
	mg_write(conn, "\r\n", 2);

	// write any body data
	if (blen > 0)
		mg_write(conn, body, blen);

	// read the response
	//memset(buf, 0, sizeof(buf));
//	LOG4CXX_DEBUG(httpclientlog, "wrote header: body len: " << blen << " content: " << body);
	int nread = 0;
	n = read_bytes(conn, buf, sizeof(buf), &nread);
	char *content = (nread > n ? ACE::strndup(buf + n, nread - n) : NULL);
	char *b = & buf[0];
	char *scode;

	if (rcnt != NULL)
		*rcnt = (size_t) (nread - n + 1);

//	LOG4CXX_DEBUG(httpclientlog, "read response: " << n << " and " << nread);
//	printf("read response: %d and %d\n", n, nread);

	// parse response
	// RFC says that all initial whitespaces should be ingored
	while (*b != '\0' && isspace(* (unsigned char *) b))
		b++;

	ri->http_version = skip(&b, " ");
	scode = skip(&b, "\r\n ");
	ri->status_code = (scode != NULL ? atoi(scode) : -1);
	skip(&b, "\r\n");

//	LOG4CXX_TRACE(httpclientlog, "http resonse " << scode << " as int: " << ri->status_code);

	if (strncmp(ri->http_version, "HTTP/", 5) == 0) {
		ri->http_version += 5;   /* Skip "HTTP/" */
		parse_http_headers(&b, ri);

#if 0
		dup_headers(ri);
		for (int i = 0; i < ri->num_headers; i++) {
			LOG4CXX_TRACE(httpclientlog, "Header:\t" << ri->http_headers[i].name <<
				"=" << ri->http_headers[i].value);
			printf("Header:\t%s=%s\n", ri->http_headers[i].name, ri->http_headers[i].value);
		}
		dispose(ri);
#endif
		if (content != NULL) {
			const char *clen = get_header(ri, "Content-Length");

			if (clen != NULL)
				*(content + atoi(clen)) = '\0';
		}
	} else {
		ri->status_code = 500;
//		LOG4CXX_WARN(httpclientlog, "cannot handle http version: " << ri->http_version);
	}

	// done with the connection
	close_connection(conn);
	free(conn);

	if (resp == NULL) {
		free(content);
	} else {
		*resp = content;
	}

	return 0;
}

const char *HttpClient::get_header(const struct mg_request_info *ri, const char *name) {
	return ::get_header(ri, name);
}

int HttpClient::parse_url(const char *url, char *host, int *port) {
	return ::parse_url(url, host, port);
}

void HttpClient::url_encode(const char *src, char *dst, size_t dst_len) {
	::url_encode(src, dst, dst_len);
}

int HttpClient::write(struct mg_connection *conn, const void *buf, size_t len) {
	return mg_write(conn, buf, len);
}

void HttpClient::close_connection(struct mg_connection *conn) {
	::close_connection(conn);
}
