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

/**
  * @file this file implements the NURBSSurface class.
  */

#pragma once

#include <vector>


#include <utils.h>
#include <vector3d.h>
#include <frame.h>


class Triangle3d;

/**
 * @class FL5LIB_EXPORT NURBSSurface
 * This class FL5LIB_EXPORT implements a 3D NURBSsurface built on an array of Frame objects.
 * The NURBS surface is described by two parameters u and v with range in [0,1].
 *
 * The NURBS control points are those of the array of Frames objects.
 * When used to describe a half body, u describes the NURBS in the x direction, and v in the z direction.
 *
*/
class FL5LIB_EXPORT NURBSSurface
{
    friend class SailNurbs;
    friend class SailDlg;

    public:
        NURBSSurface(int uAxis=0, int vAxis=2);
        NURBSSurface(NURBSSurface const &nurbs);
        ~NURBSSurface();

        void   appendFrame(Frame const &pFrame) {m_Frame.push_back(pFrame);}
        void   prependFrame(Frame const &pFrame){m_Frame.insert(m_Frame.begin(), pFrame);}
        Frame &appendNewFrame();
        void   insertFrame(const Frame &pNewFrame);
        void   insertFrame(int idx, Frame const &pFrame){m_Frame.insert(m_Frame.begin()+idx, pFrame);}
        void   clearFrames();
        int    removeFrame(int iFrame);

        Vector3d const &ctrlPoint(int iframe, int ipt) const;

        void insertPoint(int ipt);

        Frame const &frameAt(int iFrame) const {return m_Frame.at(iFrame);}
        Frame       &frame(int iFrame)         {return m_Frame[iFrame]; }
        Frame &firstFrame() {return m_Frame.front();}
        Frame const &firstFrame() const {return m_Frame.front();}
        Frame &lastFrame()  {return m_Frame.back();}
        Frame const &lastFrame() const {return m_Frame.back();}
        int    frameCount() const {return int(m_Frame.size());}
        int    nFrames() const {return int(m_Frame.size());}
        int    framePointCount() const;

        void setFrameCount(int nFrames);
        void setFramePointCount(int nPoints);

        int activeFrameIndex() const {return m_iActiveFrame;}

        void setActiveFrameIndex(int iFrame) {m_iActiveFrame = iFrame;}
        Frame &activeFrame() {return m_Frame[m_iActiveFrame];}
        const Frame &activeFrame() const {return m_Frame.at(m_iActiveFrame);}
        int removeActiveFrame() {return removeFrame(m_iActiveFrame);}

        void resizeFrames(int nSections, int nPoints);

        int indexof(Frame *pFrame) const;

        double getu(double pos, double v=0.0) const;
        double getv(double pos, double u=0.0) const;
        double getvIntersect(double u, Vector3d r) const;
        void   getPoint(double u, double v, Vector3d &Pt) const;
        void   getNormal(double u, double v, Vector3d &N) const;

        bool intersectNURBS(const Vector3d &Aa, const Vector3d &Bb, double &u, double &v, Vector3d &I) const;
        bool intersect_ref(const Vector3d &A, const Vector3d &B, double &u, double &v, Vector3d &I) const;
        bool intersect(const Vector3d &A, const Vector3d &B, double &u, double &v, Vector3d &I) const;

        int uAxis() const {return m_uAxis;}
        int VAxis() const {return m_vAxis;}
        void setUAxis(int uAxis) {m_uAxis=uAxis;}
        void setVAxis(int vAxis) {m_vAxis=vAxis;}

        void setvDegree(int nvDegree) {m_ivDegree = nvDegree;}
        void setuDegree(int nuDegree) {m_iuDegree = nuDegree;}
        int uDegree() const {return m_iuDegree;}
        int vDegree() const {return m_ivDegree;}

        double weight(const double &d, int const &i, int const &N) const ;
        void setuEdgeWeight(double uw) {m_EdgeWeightu = uw;}
        void setvEdgeWeight(double vw) {m_EdgeWeightv = vw;}
        double uEdgeWeight() const {return m_EdgeWeightu;}
        double vEdgeWeight() const {return m_EdgeWeightv;}

        void copy(NURBSSurface const & aSurface);

        void   setKnots();
        std::vector<double> const & uKnot() const {return m_uKnot;}
        std::vector<double> const & vKnot() const {return m_vKnot;}
        int uKnotCount() const {return int(m_uKnot.size());}
        int vKnotCount() const {return int(m_vKnot.size());}
        double uKnot(int idx) const {if(idx>=0 && idx<int(m_uKnot.size())) return m_uKnot.at(idx); else return 0.0;}
        double vKnot(int idx) const {if(idx>=0 && idx<int(m_vKnot.size())) return m_vKnot.at(idx); else return 0.0;}

        double hoopLength(double u, int nDiscretization);

        void setBunchParameters(double bunchamp, double bunchasym) {m_BunchAmp  = bunchamp;  m_BunchDist = bunchasym;}
        double bunchAmplitude() const {return m_BunchAmp;}
        double bunchDist() const {return m_BunchDist;}
        void setBunchAmplitude(double ba) {if(ba<0) ba=0; else if (ba>1) ba=1; m_BunchAmp=ba;}
        void setBunchDistribution(double bd) {if(bd<0) bd=0; else if (bd>1) bd=1; m_BunchDist=bd;}

        Vector3d leadingEdgeAxis() const;

        void translate(Vector3d const &T);

        bool serializeFl5(QDataStream &ar, bool bIsStoring);

        fl5Color const &color() const {return m_Color;}
        void setColor(fl5Color const &clr) {m_Color = clr;}

        void makeDefaultNurbs();

    private:
        std::vector<Frame> m_Frame;	        /**< a pointer to the array of Frame objects */

        int m_iuDegree;                 /**< the degree of the NURBS in the u direction */
        int m_ivDegree;                 /**< the degree of the NURBS in the v direction */

        std::vector<double> m_uKnot;       /**< the array of knots in the u direction */
        std::vector<double> m_vKnot;       /**< the array of knots in the v direction */

        int m_iOutput;                     /**< the number of curve points to draw the NURBS in both directions */

        double m_BunchAmp;  /** k=0.0 --> uniform bunching, k=1-->full varying bunch */
        double m_BunchDist; /** k=0.0 --> uniform bunching, k=1 weigth on endpoints */

        double m_EdgeWeightu;           /**< for a full NURBS. Unused, though, not practical */
        double m_EdgeWeightv;           /**< for a full NURBS. Unused, though, not practical */

        int m_uAxis;                    /**< used to identify along which axis parameter u is set; 0=x, 1=y, 2=z */
        int m_vAxis;                    /**< used to identify along which axis parameter u is set; 0=x, 1=y, 2=z */

        fl5Color m_Color;
        int m_iActiveFrame;


};

