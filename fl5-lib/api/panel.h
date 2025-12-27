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
#include <node.h>
#include <cartesianframe.h>
#include <enums_objects.h>
#include <panelprecision.h>


class Surface;

class FL5LIB_EXPORT Panel
{
    public:
        Panel();

        double area() const {return m_Area;}
        double minSize() const {return m_MaxSize;}

        void setSurfacePosition(xfl::enumSurfacePosition pos) {m_Pos=pos;}
        xfl::enumSurfacePosition surfacePosition() const {return m_Pos;}
        bool isOppositeSurface(xfl::enumSurfacePosition pos) const;

        int surfaceIndex() const {return m_SurfaceIndex;}
        void setSurfaceIndex(int idx) {m_SurfaceIndex=idx;}

        bool isTopPanel() const  {return m_Pos==xfl::TOPSURFACE;}
        bool isMidPanel() const  {return m_Pos==xfl::MIDSURFACE;}
        bool isBotPanel() const  {return m_Pos==xfl::BOTSURFACE;}
        bool isSidePanel() const {return m_Pos==xfl::SIDESURFACE;}
        bool isFusePanel() const {return m_Pos==xfl::FUSESURFACE;}
        bool isWakePanel() const {return m_Pos==xfl::WAKESURFACE;}

        bool isWingPanel() const {return isTopPanel() || isBotPanel() || isMidPanel() || isSidePanel();}

        void setAsWakePanel() {m_Pos=xfl::WAKESURFACE;}

        bool isLeading()              const {return m_bIsLeading;}
        bool isTrailing()             const {return m_bIsTrailing;}
        bool isLeftWingPanel()        const {return m_bIsLeftWingPanel;}
        virtual bool isPositiveOrientation()  const = 0;


        void setLeading(bool bLeading)              {m_bIsLeading = bLeading;}
        void setTrailing(bool bTrailing)            {m_bIsTrailing = bTrailing;}
        void setLeftWingPanel(bool bLeftWing)       {m_bIsLeftWingPanel=bLeftWing;}

        void setIndex(int idx) {m_index=idx;}
        int index() const  {return m_index;}

        virtual double width() const = 0;

        virtual bool hasVertex(int nodeIndex) const = 0;
        virtual Node const &leftTrailingNode() const = 0;
        virtual Node const &rightTrailingNode() const = 0;
        virtual Vector3d midTrailingPoint() const = 0;
        virtual double minAngle() const = 0;
        virtual Vector3d trailingVortex() const =0;

        virtual std::string properties(bool bLong=false) const = 0;

        virtual void translate(double tx, double ty, double tz) = 0;
        virtual void rotate(Vector3d const &HA, Vector3d const &Axis, double angle) = 0;

        virtual bool isPanel4() const = 0;
        virtual bool isPanel3() const = 0;

        virtual Vector3d const &ctrlPt(bool bVLM) const = 0;

        Vector3d const &normal() const {return m_Normal;}
        void setNormal(Vector3d const&N) {m_Normal=N;}
        virtual Vector3d const &CoG() const = 0;

        void setSurfaceNormal(Vector3d const&N) {m_SurfaceNormal=N;}
        Vector3d const &surfaceNormal() const {return m_SurfaceNormal;}


        void globalToLocal(Vector3d const &V, Vector3d &VLocal) const {m_CF.globalToLocal(V, VLocal);}
        Vector3d globalToLocal(Vector3d const &V) const {return m_CF.globalToLocal(V);}
        Vector3d globalToLocal(double const &Vx, double const &Vy, double const &Vz) const {return m_CF.globalToLocal(Vector3d(Vx,Vy, Vz));}
        Vector3d localToGlobal(Vector3d const &V) const {return m_CF.localToGlobal(V);}
        void localToGlobal(Vector3d const &Vl, Vector3d &Vg) const {m_CF.localToGlobal(Vl, Vg);}

        void localToGlobalPosition(double const&xl, double const &yl, double const &zl, double &XG, double &YG, double &ZG) const {m_CF.localToGlobalPosition(xl, yl, zl, XG, YG, ZG);}
        Vector3d localToGlobalPosition(Vector3d const &Pl) const {return m_CF.localToGlobalPosition(Pl);}
        void globalToLocalPosition(double const&XG, double const&YG, double const&ZG, double &xl, double &yl, double &zl) const {m_CF.globalToLocalPosition(XG, YG, ZG, xl, yl, zl);}
        Vector3d globalToLocalPosition(Vector3d const &P) const {return m_CF.globalToLocalPosition(P);}

        void setFlapPanel(bool bIsFlapPanel) {m_bFlapPanel=bIsFlapPanel;}
        bool isFlapPanel() const {return m_bFlapPanel;}

        Vector3d const &TELeftBisector()  const {return m_TELeftBisector;}
        Vector3d const &TERightBisector() const {return m_TERightBisector;}

        int iPL() const {return m_iPL;}
        int iPR() const {return m_iPR;}
        int iPU() const {return m_iPU;}
        int iPD() const {return m_iPD;}

        void setiWake(int idx) {m_iWake=idx;}
        int iWake() const {return m_iWake;}
        void setWakeColumn(int idx) {m_iWakeColumn=idx;}
        int iWakeColumn() const {return m_iWakeColumn;}

        bool isVisible() const {return m_bVisible;}
        void show()  {m_bVisible=true;}
        void setVisible(bool b) {m_bVisible=b;}

        static void setRFF(double rff) {s_RFF=rff;}
        static double RFF() {return s_RFF;}

    protected:

        Vector3d m_Normal;                  /**< the unit vector normal to the panel in global coordinates */

        double m_Area;             /**< The panel's area; */

        int m_index;             /**< index of the panel in the array; used when the panel array is re-arranged in non sequential order to reduce the matrix size in symmetrical calculations */
        int m_iWake;             /**< -1 if not followed by a wake panel, else equal to wake panel number */
        int m_iWakeColumn;       /**< index of the wake column shed by this panel, numbered from left tip to right tip, or -1 if none */

        double m_MaxSize;         /**< a measure of the panel's minimum cross length, for RFF estimations */

        int m_SurfaceIndex;

        CartesianFrame m_CF;

        Vector3d m_SurfaceNormal; /** the normal to te surface on which lies this panel */

        static double s_RFF;         /**< Minimum number of panel lengths at which the far field approximation is applicable.
                                    Practically, the error is roughly 0.5% at RFF=7*/

    public:
        bool m_bIsLeftWingPanel;          /**< true if the panel lies on the left (port) wing */
        bool m_bFlapPanel;
        bool m_bIsLeading;                /**< true if the panel is positioned on a leading edge */
        bool m_bIsTrailing;               /**< true if the panel is positioned on a trailing edge */
        bool m_bIsInSymPlane;             /**< true if the panel lies in the plane's xz plane of symetry at y=0*/
        int m_iPL;               /**< index of the panel which lies left of this panel, or -1 if none */
        int m_iPR;               /**< index of the panel which lies right of this panel, or -1 if none */
        int m_iPU;               /**< index of the panel which lies upstream of this panel, or -1 if none */
        int m_iPD;               /**< index of the panel which lies downstream of this panel, or -1 if none */
        xfl::enumSurfacePosition m_Pos;   /**< defines if the panel is positioned on a top, middle, bottom, side or body surface */
        Vector3d m_m, m_l;                    /**< the unit vectors which lie in the panel's plane. Cf. document NACA 4023 */

        Vector3d m_TELeftBisector; /** only for bottom trailing panels, gives the bisector direction between the top and bottom surfaces. Points downstream */
        Vector3d m_TERightBisector; /** only for bottom trailing panels, gives the bisector direction between the top and bottom surfaces. Points downstream */

        bool m_bVisible; /** Convenience field for use in 3d views; not stored */
};


