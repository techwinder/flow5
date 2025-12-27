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


#include <gqtriangle.h>
#include <node.h>
#include <panel.h>
#include <panelprecision.h>
#include <quaternion.h>
#include <segment3d.h>
#include <triangle3d.h>
#include <vector3d.h>
#include <vortex.h>


class FL5LIB_EXPORT Panel3 : public Panel
{
    friend class  TriMesh;

    public:
        Panel3();
        Panel3(Node const &S0, Node const &S1, Node const &S2);
        Panel3(Triangle3d const &triangle, xfl::enumSurfacePosition pos);
        virtual ~Panel3() = default;

        void setFrame();
        void setFrame(const Node &S0, const Node &S1, const Node &S2);
        void setFrame(const std::vector<Node> &nodelist, int index0, int index1, int index2);
        void setFrame(Triangle3d const &triangle, xfl::enumSurfacePosition pos);

        void makeXZsymmetric();

        inline void splitAtEdgeMidPoints(std::vector<Panel3> &splitpanels) const;
        inline void splitAtCoG(std::vector<Panel3> &splitpanels) const;
        void splitAtPt(Node const &ptinside, std::vector<Panel3> &splitpanels) const;

        /** Returns the position of the CoG, in global coordinates */
        Vector3d const &CoG() const override {return m_CoG_g;}

        Vector3d const &ctrlPt(bool bVLM) const override {if(bVLM) return m_CoG_g; else return m_CoG_g;}

        bool isPanel4() const override {return false;}
        bool isPanel3() const override {return true;}

        double width() const override;

        void translate(double tx, double ty, double tz) override;

        void moveVertex(int iv, Vector3d const&pos);

        inline Vector3d pointAtGlobal(double g0, double g1, double g2) const;
        inline Vector3d pointAtLocal(double g0, double g1, double g2) const;

        /** Returns true if the triangle is degenerate, false otherwise */
        bool isNullTriangle() const {return m_bNullTriangle;}

        void barycentricCoords(double xl, double yl, double *g) const
        {
            g[0] = m_gmat[0] + m_gmat[1]*xl+ m_gmat[2]*yl;
            g[1] = m_gmat[3] + m_gmat[4]*xl+ m_gmat[5]*yl;
            g[2] = m_gmat[6] + m_gmat[7]*xl+ m_gmat[8]*yl;
        }

        void barycentricCoords(const Vector3d &ptLocal, double *g) const{barycentricCoords(ptLocal.x, ptLocal.y, g);}

        void cartesianCoords(double *g, Vector3d &ptGlobal) const
        {
            ptGlobal.x = g[0]*m_S[0].x + g[1]*m_S[1].x + g[2]*m_S[2].x;
            ptGlobal.y = g[0]*m_S[0].y + g[1]*m_S[1].y + g[2]*m_S[2].y;
            ptGlobal.z = g[0]*m_S[0].z + g[1]*m_S[1].z + g[2]*m_S[2].z;
        }


        inline TRIANGLE::enumPointPosition pointPosition(double xl, double yl, int &iVertex, int &iEdge) const;

        void initialize();
        bool isPositiveOrientation() const override {return m_SignedArea>0.0;}

        void rotate(Vector3d const &HA, Quaternion &Qt);
        void rotate(Vector3d const &HA, Vector3d const &Axis, double angle) override;

        void scalarProductSourcePotential(Panel3 const &SourcePanel, bool bSelf, double *sp) const;
        void scalarProductDoubletPotential(Panel3 const &DoubletPanel, bool bSelf, double *sp) const;

        void scalarProductSourceVelocity(Panel3 const &SourcePanel, bool bSelf, double *sp) const;
        void scalarProductDoubletVelocity(Panel3 const &DoubletPanel, double *sp) const;


        void quadratureIntegrals(Vector3d Pt, double *I1, double *I3, double *I5) const;

        void computeMCIntegrals(Vector3d const &FieldPtGlobal, bool bSelf, double *G1, double *G3, double *G5, bool bGradients=false) const;

        void computeNFIntegrals(Vector3d const &FieldPtGlobal, double *G1, double *G3, double *G5, bool bGradients) const;
        void computeNFIntegrals_ref(Vector3d const &FieldPt, double *G1, double *G3, double *G5, bool bGradients) const;
        void sourceQuadraturePotential(Vector3d Pt, double &phi) const;
        void sourceQuadratureVelocity(Vector3d ptGlobal, Vector3d &V) const;

        void doubletQuadraturePotential(Vector3d Pt, double *phi) const;
        void doubletQuadratureVelocity(Vector3d Pt, Vector3d *V) const;

        inline double basis(double x, double y, int iBasis) const;

        inline void setBarycentricDoubletDensity(double m0, double m1, double m2);

        Vector3d vortexForce(Vector3d const &wind) const;

        void sourceN4023Potential(Vector3d const &C, bool bSelf, double &phi, double coreradius) const;
        void sourceN4023Velocity(Vector3d const &C, bool bSelf, Vector3d &Vel, double coreradius) const;
        void sourcePotential(const Vector3d &Pt, bool bSelf, double &phi) const;
        void sourceVelocity(const Vector3d &Pt, bool bSelf, Vector3d &Velocity) const;

        void doubletN4023Potential(Vector3d const &C, bool bSelf, double &phi, double coreradius, bool bUseRFF=true) const;
        void doubletN4023Velocity(Vector3d const &C, bool bSelf, Vector3d &V, double coreradius, bool bUseRFF=true) const;
        void doubletVortexVelocity(Vector3d const &C, Vector3d &V, double coreradius, Vortex::enumVortex vortexmodel, bool bUseRFF) const;
        void doubletBasisPotential(Vector3d const &ptGlobal, bool bSelf, double *phi, bool bUseRFF=true) const;
        void doubletBasisVelocity(Vector3d const &ptGlobal, Vector3d *V, bool bUseRFF=true) const;

        int nodeIndex(int i) const {return m_S[i].index();}
        inline void setVertex(int ivtx, Node const &node);
        inline void setNodeIndex(Node const &node);
        void setNodeIndex(int iVertex, int iNode) {if(iVertex>=0 && iVertex<3)	m_S[iVertex].setIndex(iNode);}
        inline void setNodeIndexes(int n0, int n1, int n2);
        bool hasVertex(int nodeIndex) const override {return nodeIndex==m_S[0].index() || nodeIndex==m_S[1].index() || nodeIndex==m_S[2].index();}
        inline bool hasVertex(Vector3d const& vtx) const;

        inline int vertexIndex(const Vector3d &vertex) const;
        int vertexIndex(int iNode) const
        {
            for(int in=0; in<3; in++)
            {
                if(m_S[in].index()==iNode) return in;
            }
            return -1;
        }

        bool isLeftSidePanel() const {return m_bIsLeftPanel;}
        void setLeftSidePanel(bool bLeft) {m_bIsLeftPanel = bLeft;}

        inline bool isEdgePoint(Vector3d const &PtGlobal) const;
        inline bool isPointInTriangle(Vector3d const &PtLocal) const;
        bool intersect(Vector3d const &A, Vector3d const &U, Vector3d &I) const;

        bool isTrailingEdgeNode(int iv) const;

        Node const &leftTrailingNode() const override;
        Node const &rightTrailingNode() const override;
        inline Vector3d trailingVortex() const override;
        inline Vector3d midTrailingPoint() const override;

        Segment3d const &edge(int i) const {return m_Edge[i];}
        double qualityFactor(double &r, double &shortestEdge) const;
        inline bool isSkinny() const;
        int edgeIndex(Segment3d const &seg, double precision) const
        {
            for(int iEdge=0; iEdge<3; iEdge++)
            {
                if(m_Edge[iEdge].isSame(seg, precision)) return iEdge;
            }
            return -1;
        }

        Node const *vertices() const {return m_S;}
        Node const &vertexAt(int iVtx) const {return m_S[iVtx%3];} // so that S[4]=S[0]

        void copyConnections(Panel3 const &refp3);

        void setNeighbour(int iTriangle, int iEdge);
        inline bool hasNeighbour(int idx) const;
        int neighbour(int idx) const {return m_Neighbour[idx];}
        int const *neighbours() const {return m_Neighbour;}

        inline int neighbourCount() const;

        Segment3d const &neighbourEdge(int in) const {return m_Edge[in];}

        inline bool isSame(Panel3 const &p3) const;

        double angle(int iVtx) const {return m_Angle[iVtx];}
        inline bool isLowAngle(double minangle) const;
        double minAngle() const override;

        double minEdgeLength() const;
        double maxEdgeLength() const;

        void scalePanel(double sx, double sy, double sz);
        inline void clearConnections();

        void setNodeNormal(int iv, Vector3d const &N) {if(iv<0||iv>2) return; else m_S[iv].setNormal(N);}
        void setNodeNormals(Vector3d const &N0, Vector3d const &N1, Vector3d const &N2) {m_S[0].setNormal(N0); m_S[1].setNormal(N1); m_S[2].setNormal(N2);}
        Vector3d nodeNormal(int iNode) const {assert(iNode>=0 && iNode<3); return m_S[iNode].normal();}

        Triangle3d triangle() const {return Triangle3d(m_S[0], m_S[1], m_S[2]);} // debug only

        std::string properties(bool bLong=false) const override;

        bool bFromSTL() const {return m_bFromSTL;}
        void setFromSTL(bool bSTL) {m_bFromSTL=bSTL;}

        int oppositeIndex() const {return m_iOppositeIndex;}
        void setOppositeIndex(int idx) {m_iOppositeIndex=idx;}

        double mu(int iVtx) const  {return m_mu[iVtx%3];}
        void setMu(int iVtx, double doubletdensity) {m_mu[iVtx%3]=doubletdensity;}

        Node const &node(int in) const {return m_S[in];}
        void setNode(int in, Node const &nd) {m_S[in]=nd;}

        static void setRFF(double rff) {s_RFF=rff;}
        static bool usingNintcheuFataMethod() {return s_bUseNintcheuFata;}
        static void setNintcheuFataMethod(bool bUse) {s_bUseNintcheuFata=bUse;}

        static void setQuadratureOrder(int order);
        static int quadratureOrder() {return s_iQuadratureOrder;}
        static void setQualityFactor(double qualityfactor) {s_Quality=qualityfactor;}

        static void makeGQCoeffs();

    public:
        Vector3d m_Sl[3];             /**< The three triangle vertices, in local coordinates */
        Vector3d m_CoG_l;              /**< the center of gravity's position in local coordinates */

    private:
        Node m_S[3];                  /**< the three triangle vertices, in global coordinates*/
        int m_Neighbour[3];         /**< the indexes of the neighbour triangles sharing one of the edge; three at most; -1 if no neighbour */

        double m_gmat[9];             /**< the transformation matrix from local coordinates to barycentric coordinates;*/

        bool m_bNullTriangle;

        bool m_bIsLeftPanel;        /**< true if this panel is the left triangular panel of the quad quad */

        bool m_bFromSTL;            /**< true of the panel is part of an STL mesh */
        int m_iOppositeIndex;       /**< In the case of a trailing edge panel, is the index of the panel on the opposite surface, -1 otherwise */

        Vector3d m_CoG_g;              /**< the center of gravity's position in global coordinates */
        Vector3d m_O;                  /**< the origin of the local reference frame, in global coordinates */
        Segment3d m_Edge[3];         /**< the three sides, in global coordinates */
        Vector3d m_S01l, m_S02l, m_S12l;  /**< the three sides, in local coordinates */

        double m_SignedArea;        /**< The panel's signed area; */
        double m_Angle[3];           /** the three internal angles */

        double bx[3], by[3];              /**< the integrals of x.b_i(x,y) and y.b_i(x,y) */

        double m_mu[3];               /**< testing purposes only; linear doublet density coefs: mu = mu0 b0(x,y) + mu1 b1(x,y) + mu2 b2(x,y) */
        double m_beta[3];             /**< testing purposes only; linear doublet density coefs: mu = beta0 + x beta1 + y beta2 */

        static int s_iQuadratureOrder;
        static bool s_bUseNintcheuFata;
        static GQTriangle s_gq;     /** @todo check time gain if built on the stack */
        static double s_Quality;
};



inline void Panel3::clearConnections()
{
    for(int i=0; i<3; i++)
    {
        m_Neighbour[i]=-1;
        m_S[i].clearTriangles();
        m_S[i].clearNeighbourNodes();
    }
}


inline int Panel3::neighbourCount()  const
{
    int count=0;
    for(int i=0; i<3; i++)
    {
        if(m_Neighbour[i]>=0) count++;
    }
    return count;
}


inline bool Panel3::hasNeighbour(int idx) const
{
    for(int i=0; i<3; i++)
        if(m_Neighbour[i] == idx) return true;
    return false;
}


/** returns the vertex index from the node position */
inline int Panel3::vertexIndex(Vector3d const &vertex) const
{
    for(int in=0; in<3; in++)
    {
        if(m_S[in].isSame(vertex)) return in;
    }
    return -1;
}


inline bool Panel3::isLowAngle(double minangle) const
{
    if(fabs(m_Angle[0])<minangle) return true;
    if(fabs(m_Angle[1])<minangle) return true;
    if(fabs(m_Angle[2])<minangle) return true;
    return false;
}


inline void Panel3::setQuadratureOrder(int order)
{
    s_iQuadratureOrder=order;
    s_gq.makeCoeffs(order);
}




inline bool Panel3::hasVertex(Vector3d const &vtx) const
{
    if     (m_S[0].isSame(vtx)) return true;
    else if(m_S[1].isSame(vtx)) return true;
    else if(m_S[2].isSame(vtx)) return true;
    return false;
}


inline void Panel3::setVertex(int ivtx, Node const &node)
{
    if(ivtx>=0 && ivtx<3)
    {
        m_S[ivtx] = node;
    }
}


inline void Panel3::setNodeIndex(Node const &node)
{
    for(int i=0; i<3; i++)
    {
        if(node.isSame(m_S[i]))
        {
            m_S[i].setIndex(node.index());
            return;
        }
    }
}


inline void Panel3::setNodeIndexes(int n0, int n1, int n2)
{
    m_S[0].setIndex(n0);
    m_S[1].setIndex(n1);
    m_S[2].setIndex(n2);
}


inline void Panel3::splitAtEdgeMidPoints(std::vector<Panel3> &splitpanels) const
{
    Vector3d vtx0 = edge(0).midPoint();
    Vector3d vtx1 = edge(1).midPoint();
    Vector3d vtx2 = edge(2).midPoint();
    splitpanels.clear();
    splitpanels.push_back({vtx0, m_S[2], vtx1});
    splitpanels.push_back({vtx1, m_S[0], vtx2});
    splitpanels.push_back({vtx2, m_S[1], vtx0});
    splitpanels.push_back({vtx0, vtx1, vtx2});
}


inline void Panel3::splitAtCoG(std::vector<Panel3> &splitpanels) const
{
    splitpanels.clear();
    splitpanels.push_back({m_S[0], m_CoG_g, m_S[1]});
    splitpanels.push_back({m_S[1], m_CoG_g, m_S[2]});
    splitpanels.push_back({m_S[2], m_CoG_g, m_S[0]});
}


/**
 * A triangle is said to be skinny if the circumradius-to-shortest edge ratio is greater than B
 */
inline bool Panel3::isSkinny() const
{
    double r=0.0, e=0.0;
    return qualityFactor(r,e) > s_Quality;
}


inline Vector3d Panel3::trailingVortex() const
{
    return rightTrailingNode() - leftTrailingNode();
}


/** @todo unchecked */
inline Vector3d Panel3::midTrailingPoint() const
{
    if(!isTrailing()) return Vector3d();
    if(isWingPanel())
    {
        return (m_S[1]+m_S[2])/2.0;
    }
    else if(!isWakePanel())
    {
        if(isLeftWingPanel())  return (m_S[2]+m_S[0])/2.0;
        else                   return (m_S[1]+m_S[0])/2.0;
    }
    else return Vector3d();
}


/**
 * @brief evaluates the value of a basis function as a function of Cartesian coordinates
 * @param x the x-coordinate
 * @param y the y-coordinate
 * @param iBasis the 0-based index of the basis function
 * @return the value of the basis function at point (x,y)
 */
inline double Panel3::basis(double x, double y, int iBasis) const
{
    switch(iBasis)
    {
        case 0:
        {
            double det =  ((m_Sl+1)->x*(m_Sl+2)->y - (m_Sl+2)->x*(m_Sl+1)->y)
                         -(        x*(m_Sl+2)->y - (m_Sl+2)->x*y)
                         +(        x*(m_Sl+1)->y - (m_Sl+1)->x*y);
            return det/m_SignedArea/2.0;//normalize the barycentric coordinates
        }
        case 1:
        {
            double det = (    x*(m_Sl+2)->y - (m_Sl+2)->x*y)
                        -(m_Sl->x*(m_Sl+2)->y - (m_Sl+2)->x*m_Sl->y)
                        +(m_Sl->x*y         -         x*m_Sl->y);
            return det/m_SignedArea/2.0;//normalize the barycentric coordinates
        }
        case 2:
        {
            double det = ((m_Sl+1)->x*y         -         x*(m_Sl+1)->y)
                        -(    m_Sl->x*y         -         x*m_Sl->y)
                        +(    m_Sl->x*(m_Sl+1)->y - (m_Sl+1)->x*m_Sl->y);
            return det/m_SignedArea/2.0;//normalize the barycentric coordinates
        }
    }
    return 0.0;
}


/**
 * @brief isPointInTriangle
 * @param ptGlobal the evaluation point, in global coordinates
 * @return true if the point is inside the triangular panel
 */
inline bool Panel3::isPointInTriangle(Vector3d const &ptGlobal) const
{
    double g[] = {0,0,0};
    Vector3d ptLocal = globalToLocalPosition(ptGlobal);
    if(fabs(ptLocal.z)>INPLANEPRECISION) return false;
    barycentricCoords(ptLocal, g);
    return (g[0]>=0.0 && g[1]>=0.0 && g[2]>=0.0);
}


/** Returns a point in global coordinates from its barycentric coordinates */
inline Vector3d Panel3::pointAtGlobal(double g0, double g1, double g2) const
{
    return Vector3d(g0*m_S[0].x + g1*m_S[1].x + g2*m_S[2].x,
            g0*m_S[0].y + g1*m_S[1].y + g2*m_S[2].y,
            g0*m_S[0].z + g1*m_S[1].z + g2*m_S[2].z);
}


/** Returns a point in local coordinates from its barycentric coordinates */
inline Vector3d Panel3::pointAtLocal(double g0, double g1, double g2) const
{
    return Vector3d(g0*m_Sl[0].x + g1*m_Sl[1].x + g2*m_Sl[2].x,
            g0*m_Sl[0].y + g1*m_Sl[1].y + g2*m_Sl[2].y,
            g0*m_Sl[0].z + g1*m_Sl[1].z + g2*m_Sl[2].z);
}


/** Return true if the specified panels has same node locations as this panel, false otherwise */
inline bool Panel3::isSame(Panel3 const &p3) const
{
    if(m_S[0].isSame(p3.vertexAt(0)))
    {
        if(m_S[1].isSame(p3.vertexAt(1)))
        {
            if(m_S[2].isSame(p3.vertexAt(2))) return true;
        }
        else if(m_S[2].isSame(p3.vertexAt(1)))
        {
            if(m_S[1].isSame(p3.vertexAt(2))) return true;
        }
    }
    else if(m_S[1].isSame(p3.vertexAt(0)))
    {
        if(m_S[0].isSame(p3.vertexAt(1)))
        {
            if(m_S[2].isSame(p3.vertexAt(2))) return true;
        }
        else if(m_S[2].isSame(p3.vertexAt(1)))
        {
            if(m_S[2].isSame(p3.vertexAt(0))) return true;
        }
    }
    else if(m_S[2].isSame(p3.vertexAt(0)))
    {
        if(m_S[0].isSame(p3.vertexAt(1)))
        {
            if(m_S[1].isSame(p3.vertexAt(2))) return true;
        }
        else if(m_S[1].isSame(p3.vertexAt(1)))
        {
            if(m_S[0].isSame(p3.vertexAt(2))) return true;
        }
    }
    return false;
}


#define PREC 0.00000001

/**
 * RECOMMENDED WAY TO TEST POINT POSITION
 * USING Nintcheu Fata NUMBERING FOR EDGES, NOT VERTEX BASED
 * i.e. edge 0 is between vertices 0 and 1
 *      edge 1 is between vertices 1 and 2
 *      edge 2 is between vertices 2 and 0
 */
inline TRIANGLE::enumPointPosition Panel3::pointPosition(double xl, double yl, int &iVertex, int &iEdge) const
{
    iVertex = iEdge = -1;
    double g[] = {0,0,0};
    barycentricCoords(xl, yl, g);
    if ((g[0]>PREC) && (g[0]<1.0-PREC) && (g[1]>PREC) && (g[1]<1.0-PREC)  && (g[2]>PREC) && (g[2]<1.0-PREC))
    {
        return TRIANGLE::Inside;
    }
    else if (fabs(g[0]-1.0)<PREC && fabs(g[1])<PREC && fabs(g[2])<PREC)
    {
        iVertex = 0;
        return TRIANGLE::OnVertex;
    }
    else if (fabs(g[0])<PREC && fabs(g[1]-1)<PREC && fabs(g[2])<PREC)
    {
        iVertex = 1;
        return TRIANGLE::OnVertex;
    }
    else if (fabs(g[0])<PREC && fabs(g[1])<PREC && fabs(g[2]-1)<PREC)
    {
        iVertex = 2;
        return TRIANGLE::OnVertex;
    }
    else if(fabs(g[0])<PREC && g[1]>0.0 && g[2]>0.0)
    {
        iEdge = 1;
        return TRIANGLE::OnEdge;
    }
    else if(fabs(g[1])<PREC && g[0]>0.0 && g[2]>0.0)
    {
        iEdge = 2;
        return TRIANGLE::OnEdge;
    }
    else if(fabs(g[2])<PREC && g[0]>0.0 && g[1]>0.0)
    {
        iEdge = 0;
        return TRIANGLE::OnEdge;
    }

    return TRIANGLE::Outside;
}


inline void Panel3::setBarycentricDoubletDensity(double m0, double m1, double m2)
{
    m_mu[0]=m0; m_mu[1]=m1;	m_mu[2]=m2;

    //testing only: set doublet density coefs in Cartesian coordinates.
    double g1[9];
    g1[0] = 1.0;     g1[1] = m_Sl[0].x;     g1[2] = m_Sl[0].y;
    g1[3] = 1.0;     g1[4] = m_Sl[1].x;     g1[5] = m_Sl[1].y;
    g1[6] = 1.0;     g1[7] = m_Sl[2].x;     g1[8] = m_Sl[2].y;

    matrix::invert33(g1);
    matrix::matVecMultLapack(g1,m_mu,m_beta,3,3);
}
