/****************************************************************************

    flow5
    Copyright (C) 2025 André Deperrois 
    
    This file is part of flow5.

    flow5 is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    flow5 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flow5.
    If not, see <https://www.gnu.org/licenses/>.


*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include <QDir>


#include <xflfoil/objects2d/objects2d.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflobjects/objects3d/inertia/pointmass.h>
#include <xflgeom/geom3d/frame.h>

#include "xmlxplanereader.h"

XmlXPlaneReader::XmlXPlaneReader(QFile &XFile) : XflXmlReader (XFile)
{

}





