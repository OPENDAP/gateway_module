/*
 * curl_utils.cc
 *
 *  Created on: Feb 25, 2013
 *      Author: ndp
 */


#include <GNURegex.h>


#include "util.h"
#include "BESDebug.h"
#include "GatewayUtils.h"

#include "curl_utils.h"

namespace gateway {



#define CLIENT_ERR_MIN 400
#define CLIENT_ERR_MAX 417
const char *http_client_errors[CLIENT_ERR_MAX - CLIENT_ERR_MIN +1] =
    {
        "Bad Request:",
        "Unauthorized: Contact the server administrator.",
        "Payment Required.",
        "Forbidden: Contact the server administrator.",
        "Not Found: The data source or server could not be found.\n\
        Often this means that the OPeNDAP server is missing or needs attention;\n\
        Please contact the server administrator.",
        "Method Not Allowed.",
        "Not Acceptable.",
        "Proxy Authentication Required.",
        "Request Time-out.",
        "Conflict.",
        "Gone:.",
        "Length Required.",
        "Precondition Failed.",
        "Request Entity Too Large.",
        "Request URI Too Large.",
        "Unsupported Media Type.",
        "Requested Range Not Satisfiable.",
        "Expectation Failed."
    };

#define SERVER_ERR_MIN 500
#define SERVER_ERR_MAX 505
const char *http_server_errors[SERVER_ERR_MAX - SERVER_ERR_MIN + 1] =
    {
        "Internal Server Error.",
        "Not Implemented.",
        "Bad Gateway.",
        "Service Unavailable.",
        "Gateway Time-out.",
        "HTTP Version Not Supported."
    };

/** This function translates the HTTP status codes into error messages. It
    works for those code greater than or equal to 400. */
string http_status_to_string(int status)
{
    if (status >= CLIENT_ERR_MIN && status <= CLIENT_ERR_MAX)
        return string(http_client_errors[status - CLIENT_ERR_MIN]);
    else if (status >= SERVER_ERR_MIN && status <= SERVER_ERR_MAX)
        return string(http_server_errors[status - SERVER_ERR_MIN]);
    else
        return string("Unknown Error: This indicates a problem with libdap++.\nPlease report this to support@opendap.org.");
}



// Set this to 1 to turn on libcurl's verbose mode (for debugging).
int www_trace = 0;


/** Functor to add a single string to a curl_slist. This is used to transfer
    a list of headers from a vector<string> object to a curl_slist. */
class BuildHeaders : public std::unary_function<const string &, void>
{
    struct curl_slist *d_cl;

public:
    BuildHeaders() : d_cl(0)
    {}

    void operator()(const string &header)
    {
        BESDEBUG("gateway", "BuildHeaders::operator() - Adding '" << header.c_str() << "' to the header list." << endl);
        d_cl = curl_slist_append(d_cl, header.c_str());
    }

    struct curl_slist *get_headers()
    {
        return d_cl;
    }
};





/** Use libcurl to dereference a URL. Read the information referenced by \c
    url into the file pointed to by \c stream.

    @param url The URL to dereference.
    @param stream The destination for the data; the caller can assume that
    the body of the response can be found by reading from this pointer. A
    value/result parameter
    @param resp_hdrs Value/result parameter for the HTTP Response Headers.
    @param headers A pointer to a vector of HTTP request headers. Default is
    null. These headers will be appended to the list of default headers.
    @return The HTTP status code.
    @exception Error Thrown if libcurl encounters a problem; the libcurl
    error message is stuffed into the Error object. */

long read_url(CURL *curl,
              const string &url,
              FILE *stream,
              vector<string> *resp_hdrs,
              const vector<string> *headers,
              char error_buffer[])
{

    BESDEBUG("gateway", "RemoteResource::read_url() - BEGIN" << endl);


    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

#ifdef WIN32
    //  See the curl documentation for CURLOPT_FILE (aka CURLOPT_WRITEDATA)
    //  and the CURLOPT_WRITEFUNCTION option.  Quote: "If you are using libcurl as
    //  a win32 DLL, you MUST use the CURLOPT_WRITEFUNCTION option if you set the
    //  CURLOPT_WRITEDATA option or you will experience crashes".  At the root of
    //  this issue is that one should not pass a FILE * to a windows DLL.  Close
    //  inspection of libcurl yields that their default write function when using
    //  the CURLOPT_WRITEDATA is just "fwrite".
    curl_easy_setopt(d_curl, CURLOPT_FILE, stream);
    curl_easy_setopt(d_curl, CURLOPT_WRITEFUNCTION, &fwrite);
#else
    curl_easy_setopt(curl, CURLOPT_FILE, stream);
#endif

    //DBG(copy(d_request_headers.begin(), d_request_headers.end(), ostream_iterator<string>(cerr, "\n")));

    BuildHeaders req_hdrs;
    //req_hdrs = for_each(d_request_headers.begin(), d_request_headers.end(),
     //                   req_hdrs);
    if (headers)
        req_hdrs = for_each(headers->begin(), headers->end(), req_hdrs);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, req_hdrs.get_headers());


    // Pass save_raw_http_headers() a pointer to the vector<string> where the
    // response headers may be stored. Callers can use the resp_hdrs
    // value/result parameter to get the raw response header information .
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, resp_hdrs);

    // This call is the one that makes curl go get the thing.
    CURLcode res = curl_easy_perform(curl);

    // Free the header list and null the value in d_curl.
    curl_slist_free_all(req_hdrs.get_headers());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, 0);


    if (res != 0){
        BESDEBUG("gateway", "RemoteResource::read_url() - OUCH! CURL returned an error! curl msg:  " << curl_easy_strerror(res) << endl);
        BESDEBUG("gateway", "RemoteResource::read_url() - OUCH! CURL returned an error! error_buffer:  " << error_buffer << endl);
        throw libdap::Error(error_buffer);
    }

    long status;
    res = curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &status);
    BESDEBUG("gateway", "RemoteResource::read_url() - HTTP Status " << status << endl);
    if (res != CURLE_OK)
        throw libdap::Error(error_buffer);
    BESDEBUG("gateway", "RemoteResource::read_url() - END" << endl);

    return status;
}









/**
 * Configure the proxy options for the pass curl object. The passed URL is the target URL. If the target URL
 * matches the Gateway.NoProxyRegex in the config file, then no proxying is done.
 *
 * The proxy configuration is stord in the gateway_modules configuration file, gateway.conf. The allowed values are:
 * Gateway.ProxyHost=warsaw.wonderproxy.com
 * Gateway.ProxyPort=8080
 * Gateway.ProxyUser=username
 * Gateway.ProxyPassword=password
 * Gateway.ProxyUserPW=username:password
 * Gateway.ProxyAuthType=basic | digest | ntlm
 *
 */
bool configureProxy(CURL *curl, const string &url) {
    BESDEBUG( "gateway", "RemoteResource::configureProxy() - BEGIN." << endl);

    bool using_proxy = false;

    // I pulled this because I could never find where it was applied
    // to the curl state in HTTPConnect
    //string proxyProtocol = GatewayUtils::ProxyProtocol;

    string proxyHost     = GatewayUtils::ProxyHost;
    int proxyPort        = GatewayUtils::ProxyPort;
    string proxyPassword = GatewayUtils::ProxyPassword;
    string proxyUser     = GatewayUtils::ProxyUser;
    string proxyUserPW   = GatewayUtils::ProxyUserPW;
    int proxyAuthType    = GatewayUtils::ProxyAuthType;

    if (!proxyHost.empty()) {
        using_proxy = true;
        if(proxyPort==0)
            proxyPort = 8080;

        // Apparently we don't need this...
        //if(proxyProtocol.empty())
           // proxyProtocol = "http";

    }
    if (using_proxy) {
        BESDEBUG( "gateway", "RemoteResource::configureProxy() - Found proxy configuration." << endl);

        // Don't set up the proxy server for URLs that match the 'NoProxy'
        // regex set in the gateway.conf file.

        // Don't create the regex if the string is empty
        if (!GatewayUtils::NoProxyRegex.empty()) {
            BESDEBUG( "gateway", "RemoteResource::configureProxy() - Found NoProxyRegex." << endl);
            libdap::Regex r(GatewayUtils::NoProxyRegex.c_str());
            if (r.match(url.c_str(), url.length()) != -1) {
                BESDEBUG( "gateway", "RemoteResource::configureProxy() - Found NoProxy match. Regex: " << GatewayUtils::NoProxyRegex << "; Url: " << url << endl);
                using_proxy = false;
            }
        }

        if (using_proxy) {

            BESDEBUG("gateway", "RemoteResource::configureProxy() - Setting up a proxy server." << endl);
            BESDEBUG("gateway", "RemoteResource::configureProxy() - Proxy host: " << proxyHost << endl);
            BESDEBUG("gateway", "RemoteResource::configureProxy() - Proxy port: " << proxyPort << endl);

            curl_easy_setopt(curl, CURLOPT_PROXY, proxyHost.data());
            curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxyPort);

// #ifdef CURLOPT_PROXYAUTH

            // oddly "#ifdef CURLOPT_PROXYAUTH" doesn't work - even though CURLOPT_PROXYAUTH is defined and valued at 111 it
            // fails the test. Eclipse hover over the CURLOPT_PROXYAUTH symbol shows: "CINIT(PROXYAUTH, LONG, 111)",
            // for what that's worth

            // According to http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPTPROXYAUTH As of 4/21/08 only NTLM, Digest and Basic work.

#if 0
            BESDEBUG("gateway", "RemoteResource::configureProxy() - CURLOPT_PROXYAUTH       = " << CURLOPT_PROXYAUTH << endl);
            BESDEBUG("gateway", "RemoteResource::configureProxy() - CURLAUTH_BASIC          = " << CURLAUTH_BASIC << endl);
            BESDEBUG("gateway", "RemoteResource::configureProxy() - CURLAUTH_DIGEST         = " << CURLAUTH_DIGEST << endl);
            BESDEBUG("gateway", "RemoteResource::configureProxy() - CURLAUTH_DIGEST_IE      = " << CURLAUTH_DIGEST_IE << endl);
            BESDEBUG("gateway", "RemoteResource::configureProxy() - CURLAUTH_GSSNEGOTIATE   = " << CURLAUTH_GSSNEGOTIATE << endl);
            BESDEBUG("gateway", "RemoteResource::configureProxy() - CURLAUTH_NTLM           = " << CURLAUTH_NTLM << endl);
            BESDEBUG("gateway", "RemoteResource::configureProxy() - CURLAUTH_ANY            = " << CURLAUTH_ANY << endl);
            BESDEBUG("gateway", "RemoteResource::configureProxy() - CURLAUTH_ANYSAFE        = " << CURLAUTH_ANYSAFE << endl);
            BESDEBUG("gateway", "RemoteResource::configureProxy() - CURLAUTH_ONLY           = " << CURLAUTH_ONLY << endl);
            BESDEBUG("gateway", "RemoteResource::configureProxy() - Using CURLOPT_PROXYAUTH = " << proxyAuthType << endl);
#endif

            BESDEBUG("gateway", "RemoteResource::configureProxy() - Using CURLOPT_PROXYAUTH = " << getCurlAuthTypeName(proxyAuthType) << endl);
            curl_easy_setopt(curl, CURLOPT_PROXYAUTH, proxyAuthType);
// #endif



            if (!proxyUser.empty()){
                curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, proxyUser.data());
                BESDEBUG("gateway", "RemoteResource::configureProxy() - CURLOPT_PROXYUSER : " << proxyUser << endl);

                if (!proxyPassword.empty()){
                    curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, proxyPassword.data());
                    BESDEBUG("gateway", "RemoteResource::configureProxy() - CURLOPT_PROXYPASSWORD: " << proxyPassword << endl);
                }
            }
            else if (!proxyUserPW.empty()){
                BESDEBUG("gateway",
                        "RemoteResource::configureProxy() - CURLOPT_PROXYUSERPWD : " << proxyUserPW << endl);
                curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxyUserPW.data());
            }




        }
    }
    BESDEBUG( "gateway", "RemoteResource::configureProxy() - END." << endl);

    return using_proxy;
}

string getCurlAuthTypeName(const int authType){

    string authTypeString;
    int match;

    match = authType & CURLAUTH_BASIC;
    if(match){
        authTypeString += "CURLAUTH_BASIC";
    }

    match = authType & CURLAUTH_DIGEST;
    if(match){
        if(!authTypeString.empty())
            authTypeString += " ";
        authTypeString += "CURLAUTH_DIGEST";
    }

    match = authType & CURLAUTH_DIGEST_IE;
    if(match){
        if(!authTypeString.empty())
            authTypeString += " ";
        authTypeString += "CURLAUTH_DIGEST_IE";
    }

    match = authType & CURLAUTH_GSSNEGOTIATE;
    if(match){
        if(!authTypeString.empty())
            authTypeString += " ";
        authTypeString += "CURLAUTH_GSSNEGOTIATE";
    }

    match = authType & CURLAUTH_NTLM;
    if(match){
        if(!authTypeString.empty())
            authTypeString += " ";
        authTypeString += "CURLAUTH_NTLM";
    }

    /*
    match = authType & CURLAUTH_ANY;
    if(match){
        if(!authTypeString.empty())
            authTypeString += " ";
        authTypeString += "CURLAUTH_ANY";
    }


    match = authType & CURLAUTH_ANY;
    if(match){
        if(!authTypeString.empty())
            authTypeString += " ";
        authTypeString += "CURLAUTH_ANYSAFE";
    }


    match = authType & CURLAUTH_ANY;
    if(match){
        if(!authTypeString.empty())
            authTypeString += " ";
        authTypeString += "CURLAUTH_ONLY";
    }

*/
    return authTypeString;

}








/** A libcurl callback function used to read response headers. Read headers,
    line by line, from ptr. The fourth param is really supposed to be a FILE
    *, but libcurl just holds the pointer and passes it to this function
    without using it itself. I use that to pass in a pointer to the
    HTTPConnect that initiated the HTTP request so that there's some place to
    dump the headers. Note that this function just saves the headers in a
    vector of strings. Later on the code (see fetch_url()) parses the headers
    special to the DAP.

    @param ptr A pointer to one line of character data; one header.
    @param size Size of each character (nominally one byte).
    @param nmemb Number of bytes.
    @param resp_hdrs A pointer to a vector<string>. Set in read_url.
    @return The number of bytes processed. Must be equal to size * nmemb or
    libcurl will report an error. */

size_t save_raw_http_headers(void *ptr, size_t size, size_t nmemb, void *resp_hdrs)
{
    BESDEBUG("gateway",  "curl_utils::save_raw_http_headers() - Inside the header parser." << endl);
    vector<string> *hdrs = static_cast<vector<string> * >(resp_hdrs);

    // Grab the header, minus the trailing newline. Or \r\n pair.
    string complete_line;
    if (nmemb > 1 && *(static_cast<char*>(ptr) + size * (nmemb - 2)) == '\r')
        complete_line.assign(static_cast<char *>(ptr), size * (nmemb - 2));
    else
        complete_line.assign(static_cast<char *>(ptr), size * (nmemb - 1));

    // Store all non-empty headers that are not HTTP status codes
    if (complete_line != "" && complete_line.find("HTTP") == string::npos) {
        BESDEBUG("gateway",  "curl_utils::save_raw_http_headers() - Header line: " << complete_line << endl);
        hdrs->push_back(complete_line);
    }

    return size * nmemb;
}




/** A libcurl callback for debugging protocol issues. */
int curl_debug(CURL *, curl_infotype info, char *msg, size_t size, void  *)
{
    string message(msg, size);

    switch (info) {
    case CURLINFO_TEXT:
        BESDEBUG("gateway", "RemoteResource::curl_debug() - Text: " << message << endl ); break;
    case CURLINFO_HEADER_IN:
        BESDEBUG("gateway", "RemoteResource::curl_debug() - Header in: " << message << endl ); break;
    case CURLINFO_HEADER_OUT:
        BESDEBUG("gateway", "RemoteResource::curl_debug() - Header out: " << endl << message << endl ); break;
    case CURLINFO_DATA_IN:
        BESDEBUG("gateway", "RemoteResource::curl_debug() - Data in: " << message << endl ); break;
    case CURLINFO_DATA_OUT:
        BESDEBUG("gateway", "RemoteResource::curl_debug() - Data out: " << message << endl ); break;
    case CURLINFO_END:
        BESDEBUG("gateway", "RemoteResource::curl_debug() - End: " << message << endl ); break;
#ifdef CURLINFO_SSL_DATA_IN
    case CURLINFO_SSL_DATA_IN:
        BESDEBUG("gateway", "RemoteResource::curl_debug() - SSL Data in: " << message << endl ); break;
#endif
#ifdef CURLINFO_SSL_DATA_OUT
    case CURLINFO_SSL_DATA_OUT:
        BESDEBUG("gateway", "RemoteResource::curl_debug() - SSL Data out: " << message << endl ); break;
#endif
    default:
        BESDEBUG("gateway", "RemoteResource::curl_debug() - Curl info: " << message << endl ); break;
    }
    return 0;
}





#if 0       // This code was from HTTPConnect and I don't think it's relevant for the gateway...
            // Why? Because we don't need the Gateway reading .dodsrc files. That will make our users crazy!

libdap::RCReader *RemoteResource::getRCReader(const string &url){
    BESDEBUG( "gateway", "RemoteResource::getRCReader() - Building RCReader" << endl );

    libdap::RCReader *rcr = libdap::RCReader::instance() ;

    // Don't set up the proxy server for URLs that match the 'NoProxy'
    // regex set in the gateway.conf file.
    bool configure_proxy = true;
    // Don't create the regex if the string is empty
    if (!GatewayUtils::NoProxyRegex.empty())
    {
        libdap::Regex r(GatewayUtils::NoProxyRegex.c_str());
        if (r.match(url.c_str(), url.length()) != -1) {
            BESDEBUG( "gateway", "Gateway found NoProxy match. Regex: " <<  GatewayUtils::NoProxyRegex <<  "; Url: " << url << endl );
            configure_proxy = false;
        }
    }

    if (configure_proxy)
    {
        rcr->set_proxy_server_protocol(GatewayUtils::ProxyProtocol);
        rcr->set_proxy_server_host(GatewayUtils::ProxyHost);
        rcr->set_proxy_server_port(GatewayUtils::ProxyPort);
    }

    // GatewayUtils::useInternalCache defaults to false; use squid...
    // rcr->set_use_cache( GatewayUtils::useInternalCache ) ;

    // Always use a cache!
    rcr->set_use_cache(true);

    return rcr;
}
#endif









CURL *www_lib_init(string url, char *error_buffer)
{


    CURL *curl = curl_easy_init();
    if (!curl)
        throw libdap::InternalErr(__FILE__, __LINE__, "Could not initialize libcurl.");

    if(configureProxy(curl, url))


#if 0  // Moved to RemoteResource::configureProxy()
    // ###########################################################################################
    // Now set options that will remain constant for the duration of this
    // CURL object.

    // Set the proxy host.
    if (!d_rcr->get_proxy_server_host().empty()) {
        BESDEBUG("gateway", "RemoteResource::www_lib_init() - Setting up a proxy server." << endl);
        BESDEBUG("gateway", "RemoteResource::www_lib_init() - Proxy host: " << d_rcr->get_proxy_server_host() << endl);
        BESDEBUG("gateway", "RemoteResource::www_lib_init() - Proxy port: " << d_rcr->get_proxy_server_port() << endl);
        BESDEBUG("gateway", "RemoteResource::www_lib_init() - Proxy pwd : " << d_rcr->get_proxy_server_userpw() << endl);
        curl_easy_setopt(curl, CURLOPT_PROXY,
                         d_rcr->get_proxy_server_host().c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPORT,
                         d_rcr->get_proxy_server_port());

    // As of 4/21/08 only NTLM, Digest and Basic work.
#ifdef CURLOPT_PROXYAUTH
        curl_easy_setopt(curl, CURLOPT_PROXYAUTH, (long)CURLAUTH_ANY);
#endif

        // Password might not be required. 06/21/04 jhrg
        if (!d_rcr->get_proxy_server_userpw().empty())
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD,
                             d_rcr->get_proxy_server_userpw().c_str());
    }
    // ###########################################################################################
#endif


    // Load in the default headers to send with a request. The empty Pragma
    // headers overrides libcurl's default Pragma: no-cache header (which
    // will disable caching by Squid, etc.).

    // the empty Pragma never appears in the outgoing headers when this isn't present
    // d_request_headers->push_back(string("Pragma: no-cache"));

    // d_request_headers->push_back(string("Cache-Control: no-cache"));

    // Allow compressed responses. Sending an empty string enables all supported compression types.
#ifndef CURLOPT_ACCEPT_ENCODING
    curl_easy_setopt(curl, CURLOPT_ENCODING, "");
#else
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
#endif

    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
    // We have to set FailOnError to false for any of the non-Basic
    // authentication schemes to work. 07/28/03 jhrg
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 0);

    // This means libcurl will use Basic, Digest, GSS Negotiate, or NTLM,
    // choosing the the 'safest' one supported by the server.
    // This requires curl 7.10.6 which is still in pre-release. 07/25/03 jhrg
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);

    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, save_raw_http_headers);
    // In read_url a call to CURLOPT_WRITEHEADER is used to set the fourth
    // param of save_raw_http_headers to a vector<string> object.

    // Follow 302 (redirect) responses
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);


    // Set the user agent to curls version response because, well, that's what command line curl does :)
    curl_easy_setopt(curl, CURLOPT_USERAGENT, curl_version());



#if 0
    // If the user turns off SSL validation...
    if (!d_rcr->get_validate_ssl() == 0) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    }

    // Look to see if cookies are turned on in the .dodsrc file. If so,
    // activate here. We honor 'session cookies' (cookies without an
    // expiration date) here so that session-base SSO systems will work as
    // expected.
    if (!d_cookie_jar.empty()) {
        BESDEBUG(cerr << "Setting the cookie jar to: " << d_cookie_jar << endl);
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, d_cookie_jar.c_str());
        curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);
    }
#endif


    if (www_trace) {
        BESDEBUG("gateway", "RemoteResource::www_lib_init() - Curl version: " << curl_version() << endl);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        BESDEBUG("gateway", "RemoteResource::www_lib_init() - Curl in verbose mode."<< endl);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, curl_debug);
        BESDEBUG("gateway", "RemoteResource::www_lib_init() - Curl debugging function installed."<< endl);
    }


    BESDEBUG("gateway", "RemoteResource::www_lib_init() - curl: " << curl << endl);

    return curl;


}



} /* namespace gateway */
