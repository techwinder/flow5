/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#pragma once

#include <xflpanels/panels/panel3.h>
#include <xflpanels/panels/panel4.h>
#include <xflpanels/panels/vortex.h>


void testPanel3_1();
void testPanel3_3();
void testPanel4();
void testPanel2d();
void testVortex();
void testBasisFunctions();
void testPanelNF1(bool bPositive = true);
void testPanel5();



enum PANELMETHOD {NASA4023, BASIS, VORTEX, NOPANELMETHOD};

double potentialP3(const Vector3d &pt, const QVector<Panel3> &panel3, PANELMETHOD iMethod, bool bSource, double CoreRadius);
double potentialP4(const Vector3d &pt, const QVector<Panel4> &panel4, PANELMETHOD iMethod, bool bSource, double CoreRadius);
Vector3d velocityP3(Vector3d const &pt, const QVector<Panel3> &panel3, PANELMETHOD iMethod, bool bSource, double CoreRadius, Vortex::enumVortex VortexModel);
Vector3d velocityP4(Vector3d const &pt, QVector<Panel4> const &panel4, PANELMETHOD iMethod, bool bSource, double CoreRadius);
