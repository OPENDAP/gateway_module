// GatewayContainer.cc

// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of gateway_module, A C++ module that can be loaded in to
// the OPeNDAP Back-End Server (BES) and is able to handle remote requests.

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: Patrick West <pwest@ucar.edu>
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
//      pcw       Patrick West <pwest@ucar.edu>

#include "GatewayContainer.h"

#include <BESSyntaxUserError.h>
#include <BESInternalError.h>
#include <BESDebug.h>
#include <TheBESKeys.h>

#include "GatewayRequest.h"
#include "GatewayUtils.h"
#include "GatewayResponseNames.h"

/** @brief Creates an instances of GatewayContainer with symbolic name and real
 * name, which is the remote request.
 *
 * The real_name is the remote request URL.
 *
 * @param sym_name symbolic name representing this remote container
 * @param real_name the remote request URL
 * @throws BESSyntaxUserError if the url does not validate
 * @see GatewayUtils
 */
GatewayContainer::GatewayContainer( const string &sym_name,
			            const string &real_name,
				    const string &type )
    : BESContainer( sym_name, real_name, type ), _response( 0 )
{
    if( type.empty() ) set_container_type( "gateway" ) ;
    bool found = false ;
    string key = Gateway_WHITESPACE ;
    vector<string> values ;
    TheBESKeys::TheKeys()->get_values( key, values, found ) ;
    vector<string>::const_iterator i = values.begin() ;
    vector<string>::const_iterator e = values.end() ;
    bool done = false ;
    for( ; i != e && !done; i++ )
    {
	if( (*i).length() <= real_name.length() )
	{
	    if( real_name.substr( 0, (*i).length() ) == (*i) )
	    {
		done = true ;
	    }
	}
    }
    if( !done )
    {
	string err = (string)"The specified URL " + real_name +
	             " does not match with any URLs in the whitelist" ;
	throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
    }
}

GatewayContainer::GatewayContainer( const GatewayContainer &copy_from )
    : BESContainer( copy_from ),
      _response( copy_from._response )
{
    // we can not make a copy of this container once the request has
    // been made
    if( _response )
    {
	string err = (string)"The Container has already been accessed, "
	             + "can not create a copy of this container." ;
	throw BESInternalError( err, __FILE__, __LINE__ ) ;
    }
}

void
GatewayContainer::_duplicate( GatewayContainer &copy_to )
{
    if( copy_to._response )
    {
	string err = (string)"The Container has already been accessed, "
	             + "can not duplicate this resource." ;
	throw BESInternalError( err, __FILE__, __LINE__ ) ;
    }
    copy_to._response = _response ;
    BESContainer::_duplicate( copy_to ) ;
}

BESContainer *
GatewayContainer::ptr_duplicate( )
{
    GatewayContainer *container = new GatewayContainer ;
    _duplicate( *container ) ;
    return container ;
}

GatewayContainer::~GatewayContainer()
{
    if( _response )
    {
	release() ;
    }
}

/** @brief access the remote target response by making the remote request
 *
 * @return full path to the remote request response data file
 * @throws BESError if there is a problem making the remote request
 */
string
GatewayContainer::access()
{
    BESDEBUG( "gateway", "accessing " << get_real_name() << endl );
    string type = get_container_type() ;
    if( type == "gateway" ) type = "" ;
    string accessed ;
    if( !_response )
    {
	GatewayRequest r ;
	_response = r.make_request( get_real_name(), type ) ;
	if( _response )
	    accessed = _response->get_file() ;
	set_container_type( type ) ;
    }
    else
    {
	accessed = _response->get_file() ;
    }
    BESDEBUG( "gateway", "done accessing " << get_real_name() << " returning "
		     << accessed << endl );
    BESDEBUG( "gateway", "done accessing " << *this << endl );
    return accessed ;
}

/** @brief release the resources
 *
 * Release the resource
 *
 * @return true if the resource is released successfully and false otherwise
 */
bool
GatewayContainer::release()
{
    if( _response )
    {
	BESDEBUG( "gateway", "releasing gateway response" << endl );
	delete _response ;
	_response = 0 ;
    }

    BESDEBUG( "gateway", "done releasing gateway response" << endl );
    return true ;
}

/** @brief dumps information about this object
 *
 * Displays the pointer value of this instance along with information about
 * this container.
 *
 * @param strm C++ i/o stream to dump the information to
 */
void
GatewayContainer::dump( ostream &strm ) const
{
    strm << BESIndent::LMarg << "GatewayContainer::dump - ("
			     << (void *)this << ")" << endl ;
    BESIndent::Indent() ;
    BESContainer::dump( strm ) ;
    if( _response )
    {
	strm << BESIndent::LMarg << "response file: " << _response->get_file()
	     << endl ;
	strm << BESIndent::LMarg << "response headers: " ;
	vector<string> *hdrs = _response->get_headers() ;
	if( hdrs )
	{
	    strm << endl ;
	    BESIndent::Indent() ;
	    vector<string>::const_iterator i = hdrs->begin() ;
	    vector<string>::const_iterator e = hdrs->end() ;
	    for( ; i != e; i++ )
	    {
		string hdr_line = (*i) ;
		strm << BESIndent::LMarg << hdr_line << endl ;
	    }
	    BESIndent::UnIndent() ;
	}
	else
	{
	    strm << "none" << endl ;
	}
    }
    else
    {
	strm << BESIndent::LMarg << "response not yet obtained" << endl ;
    }
    BESIndent::UnIndent() ;
}

