// GatewayUtils.h

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

#ifndef I_GatewayUtils_H
#define I_GatewayUtils_H 1

#include <string>
#include <map>
#include <vector>

using std::string ;
using std::vector ;
using std::map ;

/** @brief utility class for the gateway remote request mechanism
 *
 */
class GatewayUtils
{
public:
    static vector<string>	WhiteList ;
    static map<string,string>	MimeList ;
    static string		ProxyHost ;
    static int			ProxyPort ;

    static void			Initialize() ;

    static char *		Get_tempfile_template( char *file_template ) ;

    static void			Get_type_from_disposition( const string &disp,
						           string &type ) ;
    static void			Get_type_from_content_type( const string &ctype,
						            string &type ) ;
    static void			Get_type_from_url( const string &url,
						   string &type ) ;
} ;

#endif // I_GatewayUtils_H

