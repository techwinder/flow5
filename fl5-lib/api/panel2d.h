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

#include <vector2d.h>
#include <cartesianframe2d.h>
#include <node2d.h>


class FL5LIB_EXPORT Panel2d
{
    public:
        Panel2d();
        Panel2d(const Vector2d &P0, const Vector2d &P1);
        Panel2d(Node2d const &N0, Node2d const &N1);
        Panel2d(const Vector2d &A, const Vector2d &B, int index, bool bTEPanel);

        void setFrame(const Node2d &A, const Node2d &B, int index, bool bTEPanel);
        void setFrame(const Node2d &nd0, const Node2d &nd1);
        void setFrame();

        /** Sets the x-axis vector which defines the panel's frame */
        void setI(Vector2d const &I) {m_Frame.setI(I);}

        void linearVortexKP(Vector2d const &pt, double &psiC, double &psiL) const;
        void linearVortex(Vector2d const &pt, double &psi_p, double &psi_m) const;
        void linearVortex(Vector2d const &pt, Vector2d *dVdg1, Vector2d *dVdg2, Vector2d *d2Vdydg1_l=nullptr, Vector2d *d2Vdydg2_l=nullptr) const;

        void uniformVortex(Vector2d const &pt, Vector2d *vel) const;

        void uniformSource(Vector2d const &pt, double *phi, double *psi, Vector2d *vel) const;
        void linearSource(Vector2d const &pt, double *psi1, double *psi2, Vector2d *vel1, Vector2d *vel2) const;

        Vector2d globalToLocalVector(Vector2d const &V) const;
        Vector2d globalToLocalPosition(Vector2d const &Pos) const;
        Vector2d localToGlobalVector(Vector2d const &Vl) const;


        void trig(double x, double y, double &beta1, double &beta2, double &logr1, double &logr2) const;
        void trigXFoil(double x, double y, double &beta1, double &beta2, double &logr1, double &logr2) const;

        bool isNull() const;

        Node2d const & midPoint()  const {return I;}
        Node2d const & ctrlPoint() const {return I;}
        Vector2d const &normal() const {return Normal;}
        Vector2d tangent() const {return Vector2d(Normal.y, -Normal.x);}

        int index() const {return m_index;}
        void setIndex(int index) {m_index=index;}

        bool isTEPanel() const {return m_bTEPanel;}
        void setTEPanel(bool bTE) {m_bTEPanel=bTE;}

        bool isAirfoilPanel() const {return !m_bWakePanel;}
        bool isWakePanel() const {return m_bWakePanel;}
        void setWakePanel(bool bWake) {m_bWakePanel=bWake; A.setWakeNode(true); B.setWakeNode(true);}


        double length() const {return m_Length;}

        int upStreamBLNodeIndex() const {return m_BLNodeIndex[0];}
        int downStreamBLNodeIndex() const {return m_BLNodeIndex[1];}
        void setUpStreamBLNodeIndex(int idx) {m_BLNodeIndex[0]=idx;}
        void setDownStreamBLNodeIndex(int idx) {m_BLNodeIndex[1]=idx;}

        Vector2d const & node(int inode) const {return inode%2==0 ? A : B;}
    public:

        CartesianFrame2d m_Frame;

        int m_index;       /**< The index of the panel in the foil array of 2d panels;
                           needed because panels are rearranged in streamwise order for top and bottom BL calculations.
                           For wake panels, is the index of the panel starting with index 0 for the first trailing wake panel */

        int m_BLNodeIndex[2];      /**< the indexes of the upstream and downstream BL nodes in the BL node array. Note: NOT the indexes in the array of foil nodes */

        double m_Length;
        Node2d A, B;             /**< the two extremity points of the panel, in global coordinates*/
        Vector2d Al, Bl;         /**< the two extremity points of the panel, in local coordinates*/
        Node2d I;              /**< the middle point, in global coordinates */
        Vector2d T;              /**< the tangential vector, in global coordinates */
        Vector2d Normal;         /**< the outward normal to the panel, in global coordinates */

        double aPanel;           /**< the angle between the panel and the vertical axis, cf. Drela fig. 2 */

        bool m_bIsNull;          /**< true if the panel has zero length */
        bool m_bTEPanel;         /**< true if the panel joins the first and last nodes */
        bool m_bWakePanel;       /**< true if the panel is a wake panel */
};


