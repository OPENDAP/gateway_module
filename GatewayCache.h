/*
 * GatewayCache.h
 *
 *  Created on: Oct 2, 2015
 *      Author: ndp
 */

#ifndef MODULES_GATEWAY_MODULE_GATEWAYCACHE_H_
#define MODULES_GATEWAY_MODULE_GATEWAYCACHE_H_

#include "BESFileLockingCache.h"

namespace gateway
{


class GatewayCache: public BESFileLockingCache
{
private:
    static GatewayCache * d_instance;
    static void delete_instance() { delete d_instance; d_instance = 0; }

    GatewayCache();
    GatewayCache(const GatewayCache &src);

    static string getCacheDirFromConfig();
    static string getDimCachePrefixFromConfig();
    static unsigned long getCacheSizeFromConfig();


protected:

    // GatewayCache(const string &data_root_dir, const string &stored_results_subdir, const string &prefix, unsigned long long size);
    GatewayCache(const string &cache_dir, const string &prefix, unsigned long long size);

public:
	static const string DIR_KEY;
	static const string PREFIX_KEY;
	static const string SIZE_KEY;
	 // static const string CACHE_CONTROL_FILE;

    // static GatewayCache *get_instance(const string &bes_catalog_root_dir, const string &stored_results_subdir, const string &prefix, unsigned long long size);
    static GatewayCache *get_instance(const string &cache_dir, const string &prefix, unsigned long long size);
    static GatewayCache *get_instance();


    static string assemblePath(const string &firstPart, const string &secondPart, bool addLeadingSlash =  false);

    // Overrides parent method
    //virtual string get_cache_file_name(const string &src, bool mangle = false);

	virtual ~GatewayCache();
};


} /* namespace gateway */

#endif /* MODULES_GATEWAY_MODULE_GATEWAYCACHE_H_ */
