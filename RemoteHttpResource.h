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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

// (c) COPYRIGHT URI/MIT 1994-1999
// Please read the full copyright statement in the file COPYRIGHT_URI.
//
// Authors:
//      ndp       Nathan Potter <ndp@opendap.org>

#ifndef REMOTERESOURCE_H_
#define REMOTERESOURCE_H_


#include <curl/curl.h>
#include <curl/easy.h>

#include "util.h"
#include "BESCache3.h"
#include "InternalErr.h"
#include "RCReader.h"

using std::string;
using std::vector;

namespace gateway {


/**
 * This class encapsulates a remote resource available via HTTP GET. It willb retrieve the content of the resource and place it
 * in a local disk cache for rapid (subsequent) access. It can be configure to use a proxy server for the outgoing requests.
 */
class RemoteHttpResource {
private:


    /**
     * Resource URL that an instance of this clas represents
     */
    string d_remoteResourceUrl;

    /**
     * Open file descriptor for the resource content (Returned from the cache).
     */
    int d_fd;


    /**
     * A flag used to protect the state of the object by not allowing some method calls to be
     * made before the resource is retrieved.
     */
    bool _initialized;


    /**
     * An pointer to a CURL object to use for any HTTP transactions.
     */
    CURL *d_curl;


    /**
     * @TODO This variable fails to accumulate error message content when curl has problems. FIX.
     */
    char d_error_buffer[CURL_ERROR_SIZE]; // A human-readable message.

    /**
     * The DAP type (as utilized by the BES) of the resource. See RemoteHttpResource::setType()
     * for more.
     */
    string d_type;

    /**
     * The file name in which the content of the remote resource has been cached.
     */
    string d_resourceCacheFileName;

    /**
     * HTTP request headers added the curl HTTP GET request
     */
    vector<string> *d_request_headers; // Request headers

    /**
     * The HTTP response headers returned by the request for the remote resource.
     */
    vector<string> *d_response_headers; // Response headers


    /**
     * Determines the DAP of the remote resource. Looks at HTTP headers, and failing that compares the
     * basename in the resource URL to the data handlers TypeMatch.
     */
    void setType(const vector<string> *resp_hdrs );

    /**
     * Makes the curl call to write the resource to a file, determines DAP type of the content, and rewinds
     * the file descriptor.
     */
    void writeResourceToFile(int fd);



protected:

    RemoteHttpResource() :
        d_fd(0),
        d_curl(0),
        d_response_headers(0),
        d_request_headers(0),
        _initialized(false),
        d_resourceCacheFileName("")
    {}


public:

    RemoteHttpResource(const string &url);
    virtual ~RemoteHttpResource();

    void retrieveResource();


    /**
     * Returns the DAP type string of the RemoteHttpResource
     * @return Returns the DAP type string used by the BES Containers.
     */
    string getType() {
        return d_type;
    }

    /**
     * Returns the (read-locked) cache file name on the local system in which the content of the remote
     * resource is stored. Deleting of the instance of this class will release the read-lock.
     */
    string getCacheFileName() {
        if(!_initialized)
            throw libdap::Error("RemoteHttpResource::getCacheFileName() - STATE ERROR: Remote Resource Has Not Been Retrieved.");
        return d_resourceCacheFileName;
    }

    /**
     * Returns a vector of HTTP headers received along with the response from the request for the remote resource..
     */
    vector<string> *getResponseHeaders() {
        if(!_initialized)
            throw libdap::Error("RemoteHttpResource::getCacheFileName() - STATE ERROR: Remote Resource Has Not Been Retrieved.");
        return d_response_headers;
    }

};

} /* namespace gateway */
#endif /* REMOTERESOURCE_H_ */
