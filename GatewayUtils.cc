// GatewayUtils.cc

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

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "GatewayUtils.h"
#include "BESUtil.h"
#include "TheBESKeys.h"
#include "BESInternalError.h"
#include "GNURegex.h"
#include "util.h"

using namespace libdap ;

// Look around for a reasonable place to put a temporary file. Check first
// the value of the TMPDIR env var. If that does not yield a path that's
// writable (as defined by access(..., W_OK|R_OK)) then look at P_tmpdir (as
// defined in stdio.h. If both come up empty, then use `./'.
//
// This function allocates storage using new. The caller must delete the char
// array.

// Change this to a version that either returns a string or an open file
// descriptor. Use information from https://buildsecurityin.us-cert.gov/
// (see open()) to make it more secure. Ideal solution: get deserialize()
// methods to read from a stream returned by libcurl, not from a temporary
// file. 9/21/07 jhrg
char *
GatewayUtils::get_tempfile_template(char *file_template)
{
#ifdef WIN32
    // white list for a WIN32 directory
    Regex directory("[-a-zA-Z0-9_\\]*");

    string c = getenv("TEMP");
    if (c && directory.match(c.c_str(), c.length()) && (access(getenv("TEMP"), 6) == 0))
    	goto valid_temp_directory;

    c = getenv("TMP");
    if (c && directory.match(c.c_str(), c.length()) && (access(getenv("TEMP"), 6) == 0))
    	goto valid_temp_directory;
#else
	// white list for a directory
	Regex directory("[-a-zA-Z0-9_/]*");

	string c = getenv("TMPDIR");
	if (!c.empty() && directory.match(c.c_str(), c.length())
		&& (access(c.c_str(), W_OK | R_OK) == 0))
    	goto valid_temp_directory;

#ifdef P_tmpdir
	if (access(P_tmpdir, W_OK | R_OK) == 0) {
        c = P_tmpdir;
        goto valid_temp_directory;
	}
#endif

#endif  // WIN32

    c = ".";

valid_temp_directory:

	c.append("/");
	c.append(file_template);

    char *temp = new char[c.length()];
    strncpy(temp, c.c_str(), c.length());

    return temp;
}

