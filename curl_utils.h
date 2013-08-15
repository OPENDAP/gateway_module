// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of gateway_module, A C++ module that can be loaded in to
// the OPeNDAP Back-End Server (BES) and is able to handle remote requests.

// Copyright (c) 2013 OPeNDAP, Inc.
// Author: Nathan Potter <ndp@opendap.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

// (c) COPYRIGHT URI/MIT 1994-1999
// Please read the full copyright statement in the file COPYRIGHT_URI.
//
// Authors:
//      ndp       Nathan Potter <ndp@opendap.org>

#ifndef CURL_UTILS_H_
#define CURL_UTILS_H_


#include <curl/curl.h>
#include <curl/easy.h>

#include "util.h"
#include "BESDebug.h"



using std::vector;



namespace libcurl {


/**
 * Get's a new instance of CURL* and performs basic configuration of that instance.
 *  - Configure Gateway Proxy
 *  - Accept compressed responses
 *  - Any authentication type
 *  - Follow redirects
 *  - User agent set to curl versio.
 *
 *  @param url The url used to configure the proy.
 */
CURL *init(char *error_buffer);

/**
 * Configure the proxy options for the pass curl object. The passed URL is the target URL. If the target URL
 * matches the Gateway.NoProxyRegex in the config file, then no proxying is done.
 *
 * The proxy configuration is stored in the gateway_modules configuration file, gateway.conf. The allowed values are:
 * Gateway.ProxyHost=warsaw.wonderproxy.com
 * Gateway.ProxyPort=8080
 * Gateway.ProxyUser=username
 * Gateway.ProxyPassword=password
 * Gateway.ProxyUserPW=username:password
 * Gateway.ProxyAuthType=basic | digest | ntlm
 *
 */
bool configureProxy(CURL *curl, const string &url);

/** Use libcurl to dereference a URL. Read the information referenced by \c
    url into the file pointed to by the open file descriptor \c fd.

    @param url The URL to dereference.
    @param fd  An open file descriptor (as in 'open' as opposed to 'fopen') which
    will be the destination for the data; the caller can assume that when this
    method returns that the body of the response can be retrieved by reading
    from this file descriptor.
    @param resp_hdrs Value/result parameter for the HTTP Response Headers.
    @param request_headers A pointer to a vector of HTTP request headers. Default is
    null. These headers will be appended to the list of default headers.
    @return The HTTP status code.
    @exception Error Thrown if libcurl encounters a problem; the libcurl
    error message is stuffed into the Error object.
*/
long read_url(CURL *curl,
              const string &url,
              int fd,
              vector<string> *resp_hdrs,
              const vector<string> *headers,
              char error_buffer[]);



/** This function translates the HTTP status codes into error messages.
 * It works for those code greater than or equal to 400.
 *
 */
string http_status_to_string(int status);


} /* namespace libcurl */
#endif /* CURL_UTILS_H_ */
