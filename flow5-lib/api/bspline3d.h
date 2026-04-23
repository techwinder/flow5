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

#pragma once


#include <vector3d.h>
#include <linestyle.h>

class FL5LIB_EXPORT BSpline3d
{
    public:
        BSpline3d();

        int degree() const {return m_degree;}
        void setDegree(int deg) {m_degree=deg;}

        void appendControlPoint(double x, double y, double z, double w=1.0);
        void appendControlPoint(const Vector3d &Pt, double w=1.0);
        void resizeControlPoints(int nPts);

        void clearControlPoints() {m_CtrlPt.clear();}
        int nCtrlPoints() const {return int(m_CtrlPt.size());}
        void setControlSize(int n) {m_CtrlPt.resize(n); m_Weight.resize(n);}

        std::vector<Vector3d> const &controlPoints() const {return m_CtrlPt;}
        Vector3d &controlPoint(int i) {return m_CtrlPt[i];}
        Vector3d const &controlPoint(int i) const {return m_CtrlPt[i];}
        Vector3d const &lastCtrlPoint()     const {return m_CtrlPt.back();}
        Vector3d const &firstCtrlPoint()    const {return m_CtrlPt.front();}
        void setCtrlPoint(int n, double x, double y, double z) {m_CtrlPt[n]=Vector3d(x,y,z);}
        void setCtrlPoint(int n, Vector3d const &pt) {m_CtrlPt[n]=pt;}
        void setFirstCtrlPoint(Vector3d const&pt) {if(m_CtrlPt.size()>0) m_CtrlPt.front()=pt;}
        void setLastCtrlPoint(double x, double y, double z) {if(m_CtrlPt.size()>0) m_CtrlPt.back()=Vector3d(x,y,z);}
        void setLastCtrlPoint(Vector3d const&pt)  {if(m_CtrlPt.size()>0) m_CtrlPt.back()=pt;}
        void setCtrlPoints(const std::vector<Vector3d> &ptList, double w=1.0);

        void appendCtrlPoints(std::vector<Vector3d> const & ptList, double w=1.0);
        void appendCtrlPoints(std::vector<Vector3d> const & ptList, std::vector<double> weightlist);

        void insertCtrlPointAt(int iSel, double x, double y, double z, double w=1.0);
        void insertCtrlPointAt(int iSel, Vector3d pt, double w=1.0);
        void insertCtrlPointAt(int iSel);
        bool removeCtrlPoint(int k);
        int isCtrlPoint(double x, double y, double tolx, double toly) const;

        bool isModified() const {return m_bIsModified;}
        void setModified(bool bModified) {m_bIsModified=bModified;}

        void resetSpline();
        bool updateSpline();
        void splinePoint(double t, Vector3d &pt) const;
        void splineDerivative(double t, Vector3d &der) const;
        void splineDerivative(BSpline3d &der) const;
        void makeCurve();

        bool approximate(int degree, int nPts, std::vector<Vector3d> const& pts);

        int outputSize() const {return int(m_Output.size());}
        void setOutputSize(int n) {m_Output.resize(n);}
        std::vector<Vector3d> const &outputPts() {return m_Output;}
        Vector3d outputPt(int i) const {if(i<int(m_Output.size())) return m_Output.at(i); else return Vector3d();}

        void setTheStyle(LineStyle ls) {m_theStyle=ls;}
        LineStyle theStyle() const {return m_theStyle;}

        bool isVisible() const {return m_theStyle.m_bIsVisible;}
        void setVisible(bool bVisible) {m_theStyle.m_bIsVisible = bVisible;}

        Line::enumLineStipple stipple() const {return m_theStyle.m_Stipple;}
        void setStipple(Line::enumLineStipple style){m_theStyle.m_Stipple=style;}

        int width() const {return m_theStyle.m_Width;}
        void setWidth(int width){m_theStyle.m_Width = width;}

        void setColor(fl5Color const &clr) {m_theStyle.m_Color=clr;}
        fl5Color const &color() const {return m_theStyle.m_Color;}

        Line::enumPointStyle pointStyle() const {return m_theStyle.m_Symbol;}
        void setPointStyle(Line::enumPointStyle PointStyle) {m_theStyle.m_Symbol=PointStyle;}

        bool bShowCtrlPts() const {return m_bShowCtrlPts;}
        void showCtrlPts(bool bShow) {m_bShowCtrlPts=bShow;}

        void setKnots(std::vector<double> const&knots) {m_knot=knots;}
        std::vector<double> const &knots() const {return m_knot;}
        double knot(int ik) const {if(ik>=0 && ik<int(m_knot.size())) return m_knot.at(ik); else return 0.0;}

        void setUniformWeights();

        bool isSingular() const {return m_bSingular;}

    private:
        bool splineKnots();

    private:
        int m_iHighlight;                /**< the index of the currently highlighted control point, i.e. the point over which the mouse hovers, or -1 of none. */
        int m_iSelect;                   /**< the index of the currently selected control point, i.e. the point on which the user has last click, or -1 if none. */

        std::vector<Vector3d> m_CtrlPt;      /**< the array of the positions of the spline's control points */
        std::vector<double> m_Weight;   /**< the array of weight of control points. Used for B-Splines only. Default is 1. The higher the value, the more the curve will be pulled towards the control points. */
        std::vector<Vector3d> m_Output;      /**< the array of output points, size of which is m_iRes @todo use a QVarLengthArray or a QList*/

        LineStyle m_theStyle;

        bool m_bShowCtrlPts;
        bool m_bShowNormals;
        bool m_bIsModified;

        bool m_bClosed;
        bool m_bForcesymmetric; /** if true, the first and last control points are forced on the x-axis, i.e. y=0
                                   @todo has nothing to do here*/
        bool m_bSingular;

        double m_BunchAmp;  /** k=0.0 --> uniform bunching, k=1-->full varying bunch */
        double m_BunchDist; /** k=0.0 --> uniform bunching, k=1 weigth on endpoints */

        // BSpline specific
        std::vector<double> m_knot;            /**< the array of the values of the spline's knot */
        int m_degree;
};


