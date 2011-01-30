// GatewayRequestHandler.h

#ifndef I_GatewayRequestHandler_H
#define I_GatewayRequestHandler_H

#include "BESRequestHandler.h"

class GatewayRequestHandler : public BESRequestHandler {
public:
			GatewayRequestHandler( const string &name ) ;
    virtual		~GatewayRequestHandler( void ) ;

    virtual void	dump( ostream &strm ) const ;

    static bool		gateway_build_das( BESDataHandlerInterface &dhi ) ;
    static bool		gateway_build_dds( BESDataHandlerInterface &dhi ) ;
    static bool		gateway_build_data( BESDataHandlerInterface &dhi ) ;
    static bool		gateway_build_vers( BESDataHandlerInterface &dhi ) ;
    static bool		gateway_build_help( BESDataHandlerInterface &dhi ) ;
};

#endif // GatewayRequestHandler.h

