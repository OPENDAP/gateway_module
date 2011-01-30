// GatewayRequestHandler.cc

#include "config.h"

#include "GatewayRequestHandler.h"
#include <BESResponseHandler.h>
#include <BESResponseNames.h>
#include "GatewayResponseNames.h"
#include <BESVersionInfo.h>
#include <BESTextInfo.h>
#include "BESDapNames.h"
#include "BESDataDDSResponse.h"
#include "BESDDSResponse.h"
#include "BESDASResponse.h"
#include <BESConstraintFuncs.h>
#include <BESServiceRegistry.h>
#include <BESUtil.h>

GatewayRequestHandler::GatewayRequestHandler( const string &name )
    : BESRequestHandler( name )
{
    add_handler( DAS_RESPONSE, GatewayRequestHandler::gateway_build_das ) ;
    add_handler( DDS_RESPONSE, GatewayRequestHandler::gateway_build_dds ) ;
    add_handler( DATA_RESPONSE, GatewayRequestHandler::gateway_build_data ) ;
    add_handler( VERS_RESPONSE, GatewayRequestHandler::gateway_build_vers ) ;
    add_handler( HELP_RESPONSE, GatewayRequestHandler::gateway_build_help ) ;
}

GatewayRequestHandler::~GatewayRequestHandler()
{
}

bool
GatewayRequestHandler::gateway_build_das( BESDataHandlerInterface &dhi )
{
    bool ret = true ;
    BESResponseObject *response =
    dhi.response_handler->get_response_object();
    BESDASResponse *bdas = dynamic_cast < BESDASResponse * >(response);
    DAS *das = bdas->get_das();

    // Your code goes here

    return ret ;
}

bool
GatewayRequestHandler::gateway_build_dds( BESDataHandlerInterface &dhi )
{
    bool ret = true ;
    BESResponseObject *response =
    dhi.response_handler->get_response_object();
    BESDDSResponse *bdds = dynamic_cast < BESDDSResponse * >(response);
    DDS *dds = bdds->get_dds();

    // Your code goes here

    return ret ;
}

bool
GatewayRequestHandler::gateway_build_data( BESDataHandlerInterface &dhi )
{
    bool ret = true ;
    BESResponseObject *response =
    dhi.response_handler->get_response_object();
    BESDataDDSResponse *bdds = dynamic_cast < BESDataDDSResponse * >(response);
    DataDDS *dds = bdds->get_dds();

    // Your code goes here

    return ret ;
}

bool
GatewayRequestHandler::gateway_build_vers( BESDataHandlerInterface &dhi )
{
    bool ret = true ;
    BESVersionInfo *info = dynamic_cast<BESVersionInfo *>(dhi.response_handler->get_response_object() ) ;
    info->add_module( PACKAGE_NAME, PACKAGE_VERSION ) ;
    return ret ;
}

bool
GatewayRequestHandler::gateway_build_help( BESDataHandlerInterface &dhi )
{
    bool ret = true ;
    BESInfo *info = dynamic_cast<BESInfo *>(dhi.response_handler->get_response_object());

    // This is an example. If you had a help file you could load it like
    // this and if your handler handled the following responses.
    map<string,string> attrs ;
    attrs["name"] = PACKAGE_NAME ;
    attrs["version"] = PACKAGE_VERSION ;
    list<string> services ;
    BESServiceRegistry::TheRegistry()->services_handled( Gateway_NAME, services );
    if( services.size() > 0 )
    {
	string handles = BESUtil::implode( services, ',' ) ;
	attrs["handles"] = handles ;
    }
    info->begin_tag( "module", &attrs ) ;
    //info->add_data_from_file( "Gateway.Help", "Gateway Help" ) ;
    info->end_tag( "module" ) ;

    return ret ;
}

void
GatewayRequestHandler::dump( ostream &strm ) const
{
    strm << BESIndent::LMarg << "GatewayRequestHandler::dump - ("
			     << (void *)this << ")" << endl ;
    BESIndent::Indent() ;
    BESRequestHandler::dump( strm ) ;
    BESIndent::UnIndent() ;
}

