//# tCasarc01.cc: This program tests the Casarc interface
//# Copyright (C) 2010
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA

#include <casacore/casa/System/Casarc.h>
#include <iostream>

int main()
{
  casacore::Casarc &rc = casacore::Casarc::instance("tCasarc01_tmp.rc");

  rc.put("viewer.dpg.position.mousetools", "top");
  rc.put("viewer.dpg.position.mousetools", "right");
  rc.put("viewer.dpg.position.mousetools", "top");
  rc.put("viewer.dpg.position.mousetools", "left");

  for ( casacore::Casarc::iterator iter = rc.begin(); iter != rc.end(); ++iter ) {
    std::cout << iter->first << ": " << iter->second << std::endl;
  }
}
