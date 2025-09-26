/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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





