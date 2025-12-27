/****************************************************************************

    flow5 application
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


/** @file This file implements the Frame class FL5LIB_EXPORT used in the definition of Body objects */

#pragma once

#include <vector>
#include <QDataStream>

#include <vector3d.h>

/**
*@class FL5LIB_EXPORT Frame This class FL5LIB_EXPORT defines a frame in the yz plane, on which the body surface is built. 
 * This is similar to the way a real life body is designed and built.
 * The Frame's points may be used indiferently by a spline-type or a flat-panel-type body.
 * The Frmae's points are defined from bottom to top, i.e. for crescending z values, and for the body's left (port) side.
  * The x-value of the control points is unused, the frame's position is defined by the variable m_Position.
*/
class FL5LIB_EXPORT Frame
{
    public:
        Frame(int nCtrlPts=0);

        void    copyFrame(Frame const &frame);
        void    rotateFrameY(double Angle);
        bool    serializeFrameXfl(QDataStream &ar, bool bIsStoring);
        bool    serializeFrameXf7(QDataStream &ar, bool bIsStoring);
        void    setuPosition(int uAxis);
        void    setuPosition(int uAxis, double u);
        double  zPos() const;
        double  height() const;
        double  developedLength() const;
        double  segmentLength(int i) const;

        Vector3d const &position() const {return m_Position;}
        void setPosition(const Vector3d &Pos);
        void setPosition(double x, double y, double z) {setPosition(Vector3d(x,y,z));}
        void setXPosition(double x) {m_Position.x=x;} // need to move the control points also
        void setYPosition(double y) {m_Position.y=y;} // need to move the control points also
        void setZPosition(double z) {m_Position.z=z;} // need to move the control points also
        void moveZPosition(double dz) {m_Position.z+=dz;}

        void translate(double tx, double ty, double tz) {translate(Vector3d(tx,ty,tz));}
        void translate(Vector3d const & T);

        void scale(double ratio);

        double chord() const {return 1.0;} // redefine for NURBS sails
        double angle() const {return m_Ry;}
        void setAngle(double angle) {m_Ry=angle;}

        int     isPoint(Vector3d const &point, double deltax, double deltay, double deltaz) const;
        void    insertPoint(int n);
        void    insertPoint(int n, const Vector3d &Pt);
        int     insertPoint(Vector3d const &Real, int iAxis);
        Vector3d const &pointAt(int iPt) const {return m_CtrlPoint.at(iPt);}
        Vector3d &point(int iPt) {return m_CtrlPoint[iPt];}


        void    resizeCtrlPoints(int nPoints);
        void    clearCtrlPoints() {m_CtrlPoint.clear();}
        void    appendPoint(Vector3d const& Pt);
        void    copyPoints(const std::vector<Vector3d> &PointList);
        int     nCtrlPoints() const {return int(m_CtrlPoint.size());}
        bool    removePoint(int n);


        Vector3d selectedPoint() const {if(s_iSelect>=0 && s_iSelect<int(m_CtrlPoint.size())) return m_CtrlPoint[s_iSelect]; else return Vector3d();}
        void setSelectedPoint(Vector3d const &pt) {if(s_iSelect>=0 && s_iSelect<int(m_CtrlPoint.size())) m_CtrlPoint[s_iSelect]=pt;}

        void setCtrlPoint(int idx, Vector3d const &pt) {if(idx>=0 && idx<int(m_CtrlPoint.size())) m_CtrlPoint[idx]=pt;}
        void setCtrlPoint(int idx, double x, double y, double z) {if(idx>=0 && idx<int(m_CtrlPoint.size())) m_CtrlPoint[idx].set(x,y,z);}
        void setCtrlPoints(std::vector<Vector3d> const &ctrlpts) {m_CtrlPoint=ctrlpts;}

        const Vector3d &ctrlPointAt(int idx) const {return m_CtrlPoint.at(idx);}
        Vector3d &ctrlPoint(int idx) {return m_CtrlPoint[idx];}

        void appendCtrlPoint(Vector3d const&pt) {m_CtrlPoint.push_back(pt);}
        Vector3d const &firstControlPoint() const {return m_CtrlPoint.front();}
        Vector3d const &lastControlPoint() const  {return m_CtrlPoint.back();}

        std::vector<Vector3d> const & ctrlPoints() const {return m_CtrlPoint;}

        bool isTipFrame() const {return m_bTip;}
        void setTipFrame(bool bTip) {m_bTip=bTip;}

        static int selectedIndex() {return s_iSelect;}
        static int highlighted() {return s_iHighlight;}
        static void setHighlighted(int idx) {s_iHighlight = idx;}
        static void setSelected(int idx) {s_iSelect= idx;}


    protected:
        std::vector<Vector3d> m_CtrlPoint;	/**< the array of points which define the frame.  */
        Vector3d m_Position;             /**< the translation vector for the Frame's origin */
        bool m_bTip;
        double m_Ry; /** the rotation of the frame about the position and the y-axis. Used to describe sail sections */

        static int s_iHighlight;               /**< the point over which the mouse hovers, or -1 if none */
        static int s_iSelect;                  /**< the selected pointed, i.e. the last point on which the user has clicked, or -1 if none */
};


