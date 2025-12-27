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
 *@file
 *
 * This file defines the classes for quad panel object used both in VLM and in 3d-panel analysis
 *
 */

#pragma once

#include <quaternion.h>
#include <vector3d.h>
#include <segment3d.h>
#include <panel.h>



/**
*@class Panel4
*@brief	This class defines the quad panel object used both in VLM and in 3d-panel analysis

*	The class provides member variables which define the geometric properties of the panel, and functions used in the 3D analysis.
*
*	The name of the variables follows closely the naming used in the document NASA Contractor report 4023 "Program VSAERO Theory Document".
    Refer to this document for detailed explanations on the description of the panel and the meaning of the variables.
    The nodes are defined in a separate global array. The index of the nodes at the four corners are stored as
    member variables of this panel.
*
*	For VLM calculations, the position and length vector of the bound vortex at the panel's quarter-chord are
    stored as member variables.
*
*
*
*/
class FL5LIB_EXPORT Panel4 : public Panel
{
    public:
        inline Panel4();
        Panel4(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB);
        virtual ~Panel4() = default;
        void setPanelFrame();
        void setPanelFrame(Vector3d *pNode);
        void setPanelFrame(Vector3d const &LA, Vector3d const &LB, Vector3d const &TA, Vector3d const &TB);

        void doubletVortexVelocity(Vector3d const &C, Vector3d &VTest, double coreradius, bool bUseRFF=true) const;
        void doubletN4023Potential(Vector3d const &C, bool bSelf, double &phi, double coreradius, bool bUseRFF=true) const;
        void doubletN4023Velocity(Vector3d const &C, Vector3d &V, double coreradius, bool bUseRFF=true) const;
        void sourceN4023Potential(Vector3d const &C, double &phi, double coreradius) const;
        void sourceN4023Velocity(  Vector3d const &C, bool bSelf, Vector3d &Vel, double coreradius) const;

        void rotate(Vector3d const &HA, const Vector3d &Axis, double angle) override;

        void rotate(double alpha, double beta, double phi);

        inline void rotateNormal(Quaternion const &Qt);

        inline void reset();
        bool intersect(Vector3d const &A, Vector3d const &U, Vector3d &I) const;

        void setPositiveOrientation(bool bPositiveOrientation){m_bPositiveOrientation=bPositiveOrientation;}

        double width() const override;
        double length() const;

        Vector3d const &CoG() const override {return m_CollPt;}
        Vector3d const &ctrlPt(bool bVLM) const override {if(bVLM && isMidPanel()) return m_CtrlPt; else return m_CollPt;}

        bool isPositiveOrientation()  const override {return m_bPositiveOrientation;}

        bool isPanel4() const override {return true;}
        bool isPanel3() const override {return false;}

        bool hasVertex(int nodeIndex) const override {return nodeIndex==m_iLA || nodeIndex==m_iLB || nodeIndex==m_iTA || nodeIndex==m_iTB;}
        void setNodeIndexes(int ila, int ilb, int ita, int itb) {m_iLA=ila; m_iLB=ilb; m_iTA=ita; m_iTB=itb;}

        bool isEdgePoint(Vector3d const &PtGlobal) const;

        Node const &leftTrailingNode()  const override {return m_Node[2];} // not useful - depends on the normal
        Node const &rightTrailingNode() const override {return m_Node[1];} // not useful - depends on the normal
        Vector3d midTrailingPoint() const override {return (m_Node[1]+m_Node[2])*0.5;} // not useful - depends on the normal
        Vector3d trailingVortex() const override {return m_VB-m_VA;}
        Vector3d vortexPosition() const {return (m_VA+m_VB)/2.0;}

        std::string properties(bool bLong=false) const override;
        Node const &vertex(int iNode) const {return m_Node[iNode%4];}

        double warpAngle() const;
        double minAngle() const override;

        void translate(Vector3d translation) {translate(translation.x, translation.y, translation.z);}
        void translate(double tx, double ty, double tz) override;

        Vector3d const &LA() const {return m_Node[0];} // leading  left
        Vector3d const &TA() const {return m_Node[1];} // trailing left
        Vector3d const &TB() const {return m_Node[2];} // trailing right
        Vector3d const &LB() const {return m_Node[3];} // leading  right

        Segment3d frontEdge() const {return Segment3d(m_Node[0], m_Node[3]);}  // LA-LB
        Segment3d leftEdge()  const {return Segment3d(m_Node[0], m_Node[1]);}  // LA-TA
        Segment3d rearEdge()  const {return Segment3d(m_Node[1], m_Node[2]);}  // TA-TB
        Segment3d rightEdge() const {return Segment3d(m_Node[3], m_Node[2]);}  // LB-TB


        Vector3d vortexForce(Vector3d const &wind) const;

        static void setCtrlPtFracPos(double pos) {s_CtrlPos=pos;}
        static double ctrlPtFracPos() {return s_CtrlPos;}

        static void setVortexFracPos(double pos) {s_VortexFracPos=pos;}
        static double vortexFracPos() {return s_VortexFracPos;}


    private:
        //Local frame of refernce
        Vector3d P1;                /**< the coordinates of the panel's corners, in local coordinates */
        Vector3d P2;                /**< the coordinates of the panel's corners, in local coordinates */
        Vector3d P3;                /**< the coordinates of the panel's corners, in local coordinates */
        Vector3d P4;                /**< the coordinates of the panel's corners, in local coordinates */

        bool m_bPositiveOrientation;       /**< true if the vertices are arranged clockwise around the normal, false otherwise */

        static double s_VortexFracPos; /**< Defines the relative position of the bound vortex in the streamwise direction. Usually the vortex is positioned at the panel's quarter chord i.e. s_VortexPos=0.25 */
        static double s_CtrlPos;   /**< Defines the relative position of the panel's control point in VLM. Usually the control point is positioned at the panel's 3/4 chord : s_VortexPos=0.75 */

    public:
        int m_iLA;                 /**< index of the leading left node in the node array */
        int m_iLB;                 /**< index of the leading right node in the node array */
        int m_iTA;                 /**< index of the trailing left node in the node array */
        int m_iTB;                 /**< index of the trailing right node in the node array */
        Vector3d m_CtrlPt;            /**< the position of the control point for VLM analysis or 3D/Thin panels analysis */
        Vector3d m_CollPt;            /**< the collocation point for 3d panel analysis */
        Vector3d m_VA;                /**< the left end point of the bound quarter-chord vortex on this panel */
        Vector3d m_VB;                /**< the rightt end point of the bound quarter-chord vortex on this panel */
        double SMP, SMQ;

        Node m_Node[4];            /**< the array of the four vertices, in circular order */
};


/**
 * The public constructor
 */
inline Panel4::Panel4()
{
    reset();
}


/**
 * @param Qt the quaternion which defines the 3d rotation
 */
inline void Panel4::rotateNormal(Quaternion const &Qt)
{
    Qt.conjugate(m_Normal);
}


/**
 * Resets the panel geometry to default initialization values
 */
inline void Panel4::reset()
{
    m_bPositiveOrientation = true;

    SMP = SMQ = 0.0;

    m_iLA = m_iLB = m_iTA = m_iTB = -1;

    m_iWake       = -1;
    m_iWakeColumn = -1;
}


