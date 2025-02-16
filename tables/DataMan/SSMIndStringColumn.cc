//# SSMIndStringColumn.cc: a indirect String Array Column of the 
//# Standard Storage Manager
//# Copyright (C) 2000
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA

#include <casacore/tables/DataMan/SSMIndStringColumn.h>
#include <casacore/tables/Tables/Table.h>
#include <casacore/tables/DataMan/SSMStringHandler.h>
#include <casacore/tables/DataMan/DataManError.h>
#include <casacore/casa/Arrays/Array.h>
#include <casacore/casa/Utilities/ValType.h>
#include <casacore/casa/Utilities/Assert.h>

namespace casacore { //# NAMESPACE CASACORE - BEGIN

SSMIndStringColumn::SSMIndStringColumn (SSMBase* aParent, int aDataType, 
					uInt aColNr): 
SSMDirColumn   (aParent,aDataType,aColNr)
{
}

SSMIndStringColumn::~SSMIndStringColumn()
{
}

void SSMIndStringColumn::setShape (rownr_t aRowNr, const IPosition& aShape)
{
  DebugAssert(itsShape.nelements() == 0,AipsError);
  Int buf[3];
  // Try to find out if this value was filled before, in that case we use
  // an overwrite.
  getRowValue(buf, aRowNr);
  itsSSMPtr->getStringHandler()->putShape(buf[0], buf[1], buf[2], 
					  aShape);
  putValue(aRowNr, buf);
}

IPosition SSMIndStringColumn::shape (rownr_t aRowNr)
{
  if (itsShape.nelements() != 0) {
    return itsShape;
  }

  IPosition aShape;
  Int buf[3];

  getRowValue(buf, aRowNr);
  if (buf[2] > 0) {
    itsSSMPtr->getStringHandler()->getShape(aShape, buf[0], buf[1], 
					    buf[2]);
  } else {
    throw DataManInvOper ("SSMIndStringColumn::getShape: no array in row "+
			   String::toString(aRowNr) + " in column "
                           + columnName()
                           + " of table " + itsSSMPtr->table().tableName());
  }
  return aShape;
}

Bool SSMIndStringColumn::canChangeShape() const
{
  return itsShape.nelements() ==0;
}

Bool SSMIndStringColumn::isShapeDefined (rownr_t aRowNr)
{
  if (itsShape.nelements() != 0) {
    return True;
  } else {
    Int buf[3];
    getRowValue(buf, aRowNr);
    return buf[2] != 0;
  }
}


uInt SSMIndStringColumn::ndim (rownr_t aRowNr)
{
  return shape(aRowNr).nelements();
}

void SSMIndStringColumn::getArrayV (rownr_t aRowNr,
                                    ArrayBase& aDataPtr)
{
  if (itsShape.nelements() != 0) {
    SSMDirColumn::getArrayV (aRowNr,aDataPtr);
  } else {
    Int buf[3];
    getRowValue(buf, aRowNr);
    if ( buf[2] == 0 ) {
      throw DataManInvOper
        ("SSMIndStringColumn::getArrayStringV: no array in row "
         + String::toString(aRowNr) + " in column " + columnName()
         + " of table " + itsSSMPtr->table().tableName());
    }
    itsSSMPtr->getStringHandler()->get(static_cast<Array<String>&>(aDataPtr),
                                       buf[0], buf[1], buf[2], True);
  }
}

void SSMIndStringColumn::putArrayV (rownr_t aRowNr,
                                    const ArrayBase& aDataPtr)
{
  if (itsShape.nelements() != 0) {
    SSMDirColumn::putArrayV (aRowNr, aDataPtr);
  } else {
    Int buf[3];
    // Try to find out if this value was filled before, in that case we use
    // an overwrite.
    getRowValue(buf, aRowNr);
    itsSSMPtr->getStringHandler()->put(buf[0], buf[1], buf[2], 
				       static_cast<const Array<String>&>(aDataPtr),
                                       True);
    putValue(aRowNr, buf);
  }
}


} //# NAMESPACE CASACORE - END

