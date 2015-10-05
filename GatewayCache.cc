/*
 * GatewayCache.cc
 *
 *  Created on: Oct 2, 2015
 *      Author: ndp
 */

#include "GatewayCache.h"
#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

#include "BESInternalError.h"
#include "BESDebug.h"
#include "TheBESKeys.h"


namespace gateway
{

GatewayCache *GatewayCache::d_instance = 0;
const string GatewayCache::DIR_KEY       = "Gateway.Cache.dir";
const string GatewayCache::PREFIX_KEY    = "Gateway.Cache.prefix";
const string GatewayCache::SIZE_KEY      = "Gateway.Cache.size";


unsigned long GatewayCache::getCacheSizeFromConfig(){

	bool found;
    string size;
    unsigned long size_in_megabytes = 0;
    TheBESKeys::TheKeys()->get_value( SIZE_KEY, size, found ) ;
    if( found ) {
    	std::istringstream iss(size);
    	iss >> size_in_megabytes;
    }
    else {
    	string msg = "[ERROR] GatewayCache::getCacheSize() - The BES Key " + SIZE_KEY + " is not set! It MUST be set to utilize the NcML Dimension Cache. ";
    	BESDEBUG("cache", msg << endl);
        throw BESInternalError(msg , __FILE__, __LINE__);
    }
    return size_in_megabytes;
}

string GatewayCache::getCacheDirFromConfig(){
	bool found;
    string subdir = "";
    TheBESKeys::TheKeys()->get_value( DIR_KEY, subdir, found ) ;

	if( !found ) {
    	string msg = "[ERROR] GatewayCache::getSubDirFromConfig() - The BES Key " + DIR_KEY + " is not set! It MUST be set to utilize the NcML Dimension Cache. ";
    	BESDEBUG("cache", msg << endl);
        throw BESInternalError(msg , __FILE__, __LINE__);
	}
	else {
		while(*subdir.begin() == '/' && subdir.length()>0){
			subdir = subdir.substr(1);
		}
		// So if it's value is "/" or the empty string then the subdir will default to the root
		// directory of the BES data system.
	}

    return subdir;
}


string GatewayCache::getDimCachePrefixFromConfig(){
	bool found;
    string prefix = "";
    TheBESKeys::TheKeys()->get_value( PREFIX_KEY, prefix, found ) ;
	if( found ) {
		prefix = BESUtil::lowercase( prefix ) ;
	}
	else {
    	string msg = "[ERROR] GatewayCache::getResultPrefix() - The BES Key " + PREFIX_KEY + " is not set! It MUST be set to utilize the NcML Dimension Cache. ";
    	BESDEBUG("cache", msg << endl);
        throw BESInternalError(msg , __FILE__, __LINE__);
	}

    return prefix;
}



GatewayCache::GatewayCache()
{
	BESDEBUG("cache", "GatewayCache::GatewayCache() -  BEGIN" << endl);

	string cacheDir               = getCacheDirFromConfig();
    string cachePrefix            = getDimCachePrefixFromConfig();
    unsigned long cacheSizeMbytes = getCacheSizeFromConfig();

    BESDEBUG("cache", "GatewayCache() - Cache configuration params: " << cacheDir << ", " << cachePrefix << ", " << cacheSizeMbytes << endl);

  	initialize(cacheDir, cachePrefix, cacheSizeMbytes);

    BESDEBUG("cache", "GatewayCache::GatewayCache() -  END" << endl);

}
GatewayCache::GatewayCache(const string &cache_dir, const string &prefix, unsigned long long size){

	BESDEBUG("cache", "GatewayCache::GatewayCache() -  BEGIN" << endl);

  	initialize(cache_dir, prefix, size);

  	BESDEBUG("cache", "GatewayCache::GatewayCache() -  END" << endl);
}



GatewayCache *
GatewayCache::get_instance(const string &cache_dir, const string &result_file_prefix, unsigned long long max_cache_size)
{
    if (d_instance == 0){
        if (dir_exists(cache_dir)) {
        	try {
                d_instance = new GatewayCache(cache_dir, result_file_prefix, max_cache_size);
#ifdef HAVE_ATEXIT
                atexit(delete_instance);
#endif
        	}
        	catch(BESInternalError &bie){
        	    BESDEBUG("cache", "[ERROR] GatewayCache::get_instance(): Failed to obtain cache! msg: " << bie.get_message() << endl);
        	}
    	}
    }
    return d_instance;
}

/** Get the default instance of the GatewayCache object. This will read "TheBESKeys" looking for the values
 * of SUBDIR_KEY, PREFIX_KEY, an SIZE_KEY to initialize the cache.
 */
GatewayCache *
GatewayCache::get_instance()
{
    if (d_instance == 0) {
		try {
			d_instance = new GatewayCache();
#ifdef HAVE_ATEXIT
            atexit(delete_instance);
#endif
		}
		catch(BESInternalError &bie){
			BESDEBUG("cache", "[ERROR] GatewayCache::get_instance(): Failed to obtain cache! msg: " << bie.get_message() << endl);
		}
    }

    return d_instance;
}



GatewayCache::~GatewayCache()
{
	delete_instance();
}



#if 0

/**
 * Returns the fully qualified file system path name for the dimension cache file associated
 * with this particular cache resource..
 */
string GatewayCache::get_cache_file_name(const string &local_id, bool mangle)
{

	BESDEBUG("cache", "GatewayCache::get_cache_file_name() - Starting with local_id: " << local_id << endl);

	std::string cacheFileName(local_id);

	cacheFileName = assemblePath( getCacheFilePrefix(), cacheFileName);

	if(mangle){

		// There should never be spaces in a URL, but just in case we'll replace them anyway...
		std::replace( cacheFileName.begin(), cacheFileName.end(), ' ', '#'); // replace all ' ' with '#'

		// We know there will be slashes ('/') in URLs so we'll purge those...
		std::replace( cacheFileName.begin(), cacheFileName.end(), '/', '#'); // replace all '/' with '#'
	}

	cacheFileName = assemblePath( getCacheDirectory(),  cacheFileName,true);

	BESDEBUG("cache","GatewayCache::get_cache_file_name() - cacheFileName: " <<  cacheFileName << endl);

	return cacheFileName;
}
#endif




} /* namespace gateway */
