/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QDebug>

#include "vector2d.h"
#include <xflmath/constants.h>



void Vector2d::rotateZ(Vector2d const &O, double ZTilt)
{
    //Rotate the vector around the Z-axis, by an angle ZTilt
    Vector2d OP;
    OP.x = x-O.x;
    OP.y = y-O.y;

    ZTilt *= PI/180.0;

    x = O.x + OP.x * cos(ZTilt) - OP.y * sin(ZTilt);
    y = O.y + OP.x * sin(ZTilt) + OP.y * cos(ZTilt);
}


void Vector2d::rotateZ(double beta)
{
    beta *= PI/180.0;

    double xo = x;
    double yo = y;
    x =  xo * cos(beta) - yo * sin(beta);
    y =  xo * sin(beta) + yo * cos(beta);
}


/**
 *     Returns the angle of this vector with V, in degrees
 */
double Vector2d::angle(Vector2d const &v) const
{
    double dot = v.x*x + v.y*y; // proportional to cosine
    double det = v.x*y - v.y*x; // proportional to sine
    return atan2(det, dot) * 180.0/PI;
}


void Vector2d::displayCoords(QString msg) const
{
    QString strong, str;
    str = QString::asprintf("  %17.9g  %17.9g", x,y);
    strong = msg+ " " +str;
    qDebug()<<strong.toStdString().c_str();
}



