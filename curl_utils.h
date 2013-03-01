/*
 * curl_utils.h
 *
 *  Created on: Feb 25, 2013
 *      Author: ndp
 */

#ifndef CURL_UTILS_H_
#define CURL_UTILS_H_


#include <curl/curl.h>
#include <curl/easy.h>
#include "util.h"
#include "BESDebug.h"



using std::vector;






namespace gateway {

string http_status_to_string(int status);

bool configureProxy(CURL *curl, const string &url);



string getCurlAuthTypeName(const int authType);


long read_url(CURL *curl,
              const string &url,
              FILE *stream,
              vector<string> *resp_hdrs,
              const vector<string> *headers,
              char error_buffer[]);



size_t save_raw_http_headers(void *ptr, size_t size, size_t nmemb, void *resp_hdrs);


int curl_debug(CURL *, curl_infotype info, char *msg, size_t size, void  *);


CURL *libcurl_init(string url, char *error_buffer);





} /* namespace gateway */
#endif /* CURL_UTILS_H_ */
