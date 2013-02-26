/*
 * RemoteHttpResource.h
 *
 *  Created on: Feb 22, 2013
 *      Author: ndp
 */

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

// long read_url(CURL *curl, const string &url, FILE *stream, vector<string> *resp_hdrs, const vector<string> *headers, char error_buffer[]);

// CURL *www_lib_init(string url, char *error_buffer);

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
     * Open FILE * stream for the resource content (made from fdopen(fd))
     */
    FILE *d_fstrm;

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

    string d_type;
    string d_resourceCacheFileName;

    vector<string> *d_request_headers; // Request headers
    vector<string> *d_response_headers; // Response headers


    void setType(const vector<string> *resp_hdrs );
    FILE *writeResourceToFile(int fd);

protected:
    RemoteHttpResource();
    RemoteHttpResource(libdap::RCReader *rcr);

public:
    RemoteHttpResource(const string &url);
    virtual ~RemoteHttpResource();

    void retrieveResource();

    int getFileDescriptor(){
        return d_fd;
    }

    FILE *getFileStream(){
        return d_fstrm;
    }

    string getType() {
        return d_type;
    }

    string getCacheFileName() {
        if(!_initialized)
            throw libdap::Error("RemoteHttpResource::getCacheFileName() - STATE ERROR: Remote Resource Has Not Been Retrieved.");
        return d_resourceCacheFileName;
    }

    vector<string> *getResponseHeaders() {
        if(!_initialized)
            throw libdap::Error("RemoteHttpResource::getCacheFileName() - STATE ERROR: Remote Resource Has Not Been Retrieved.");
        return d_response_headers;
    }

    //libdap::RCReader *getRCReader(const string &url);
};

} /* namespace gateway */
#endif /* REMOTERESOURCE_H_ */
