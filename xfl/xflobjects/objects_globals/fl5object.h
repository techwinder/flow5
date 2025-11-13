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

#pragma once

/**
 * @brief The fl5Object class is the base class for planes and boats
 */


#include <xflcore/enums_objects.h>
#include <xflcore/linestyle.h>
#include <xflobjects/objects_globals/optstructures.h>
#include <xflpanels/mesh/trimesh.h>

class fl5Object
{
    public:
        fl5Object() = default;
        virtual ~fl5Object() = default;

    public:
        QString const &name() const {return m_Name;}
        void setName(QString const &name) {m_Name=name;}

        const QString & description() const {return m_Description;}
        void setDescription(QString const &description) {m_Description = description;}

        void setTheStyle(LineStyle ls)  {m_theStyle=ls;}
        LineStyle const &theStyle() const {return m_theStyle;}
        QColor const&lineColor() const {return m_theStyle.m_Color;}
        void setLineColor(QColor const &clr){m_theStyle.m_Color=clr;}

        bool isVisible() const {return m_theStyle.m_bIsVisible;}
        void setVisible(bool bVisible) {m_theStyle.m_bIsVisible=bVisible;}

        int nPanel3() const                     {return m_TriMesh.nPanels();}
        int nNodes()  const                     {return m_TriMesh.nNodes();}
        Panel3 const &panel3At(int index) const {return m_TriMesh.panelAt(index);}
        Panel3       &panel3(int index)         {return m_TriMesh.panel(index);}
        Node const &node(int index) const       {return m_TriMesh.nodeAt(index);}
        TriMesh const &triMesh() const          {return m_TriMesh;}
        TriMesh &triMesh()                      {return m_TriMesh;}
        TriMesh const & refTriMesh() const      {return m_RefTriMesh;}
        TriMesh & refTriMesh()                  {return m_RefTriMesh;}
        virtual void restoreMesh() {m_TriMesh = m_RefTriMesh;}



        Line::enumLineStipple lineStipple() const {return m_theStyle.m_Stipple;}
        void setLineStipple(Line::enumLineStipple iStipple)  {m_theStyle.m_Stipple=iStipple;}

        int lineWidth() const {return m_theStyle.m_Width;}
        void setLineWidth(int iWidth) {m_theStyle.m_Width=iWidth;}


        Line::enumPointStyle pointStyle() const {return m_theStyle.m_Symbol;}
        void setPointStyle(Line::enumPointStyle iPointStyle)  {m_theStyle.m_Symbol=iPointStyle;}

        QVector<Panel3> &triPanels()             {return m_TriMesh.panels();}
        QVector<Panel3> const &triPanels() const {return m_TriMesh.panels();}

        void setRefTriMesh(TriMesh const &trimesh) {m_RefTriMesh=trimesh;}
        void setTriMesh(   TriMesh const &trimesh) {m_TriMesh=trimesh;}

    protected:
        QString m_Name;
        QString m_Description;

        LineStyle m_theStyle;

        TriMesh m_RefTriMesh; /** The reference triangular mesh, with non-rotated panels */
        TriMesh m_TriMesh;     /** The active triangular mesh, with panels rotated with bank angle and sail angle */

    public:
        mutable QVector<OptVariable> m_OptVariables;
};


