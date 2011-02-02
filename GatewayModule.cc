// GatewayModule.cc

#include <iostream>
#include <vector>
#include <string>

using std::endl ;
using std::vector ;
using std::string ;

#include "GatewayModule.h"

#include <BESRequestHandlerList.h>
#include <BESDebug.h>
#include <BESResponseHandlerList.h>
#include <BESResponseNames.h>
#include <BESContainerStorageList.h>
#include <TheBESKeys.h>
#include <BESSyntaxUserError.h>

#include "GatewayRequestHandler.h"
#include "GatewayResponseNames.h"
#include "GatewayContainerStorage.h"

void
GatewayModule::initialize( const string &modname )
{
    BESDEBUG( modname, "Initializing Gateway Module "
		       << modname << endl ) ;

    BESDEBUG( modname, "    adding " << modname
		       << " request handler" << endl ) ;
    BESRequestHandlerList::TheList()->add_handler( modname, new GatewayRequestHandler( modname ) ) ;

    BESDEBUG( modname, "    adding " << modname << " container storage" << endl );
    BESContainerStorageList::TheList()->
	add_persistence( new GatewayContainerStorage( modname ) ) ;

    BESDEBUG( modname, "    making sure Whitelist is defined" << endl ) ;
    bool found = false ;
    string key = Gateway_WHITESPACE ;
    vector<string> values ;
    TheBESKeys::TheKeys()->get_values( key, values, found ) ;
    if( !found || values.size() == 0 )
    {
	string err = (string)"The parameter " + Gateway_WHITESPACE +
	             " is not set or has no values in the gateway" +
		     " configuration file" ;
	throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
    }

    BESDEBUG( modname, "    adding Gateway debug context" << endl ) ;
    BESDebug::Register( modname ) ;

    BESDEBUG( modname, "Done Initializing Gateway Module "
		       << modname << endl ) ;
}

void
GatewayModule::terminate( const string &modname )
{
    BESDEBUG( modname, "Cleaning Gateway module " << modname << endl ) ;

    BESDEBUG( modname, "    removing " << modname
		       << " request handler"
    << endl ) ;
    BESRequestHandler *rh = BESRequestHandlerList::TheList()->remove_handler( modname ) ;
    if( rh ) delete rh ;

    BESContainerStorageList::TheList()->deref_persistence( modname ) ;

    // TERM_END
    BESDEBUG( modname, "Done Cleaning Gateway module "
		       << modname << endl ) ;
}

extern "C"
{
    BESAbstractModule *maker()
    {
	return new GatewayModule ;
    }
}

void
GatewayModule::dump( ostream &strm ) const
{
    strm << BESIndent::LMarg << "GatewayModule::dump - ("
			     << (void *)this << ")" << endl ;
}

