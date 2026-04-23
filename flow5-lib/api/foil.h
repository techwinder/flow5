/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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



#include <cubicspline.h>
#include <linestyle.h>
#include <node2d.h>
#include <xflobject.h>


class BSpline;

class FL5LIB_EXPORT Foil : public XflObject
{
    public:
        Foil();
        Foil(const Foil *pFoil);
        Foil(const Spline *pSpline);

        Vector2d const &basePoint(int i) const {return m_BaseNode.at(i);}
        void setBasePoint(int ipt, Vector2d const& pt) {m_BaseNode[ipt]=pt;}

        Node2d node(int index) const;
        void setNode(int idx, double xnew, double ynew) {if(idx>=0 && idx<nBaseNodes()) m_BaseNode[idx]={xnew, ynew};}
        Vector2d const &normal(int i) const {return m_Node.at(i).normal();}

        void getY(std::vector<Vector2d> const &vec, double m_x, double &m_y) const;
        double baseUpperY(double m_x) const;
        double baseLowerY(double m_x) const;
        Vector2d midYRel(double sRel, Vector2d &N) const;
        Vector2d lowerYRel(double xRel, Vector2d &N) const;
        Vector2d upperYRel(double xRel, Vector2d &N) const;

        void setDefaultMidLinePointDistribution(int nMidPoints);
        void makeBaseMidLine();

        double TEGap()               const;
        double area()                const;
        double bottomSlope(double x) const;
        double camber(double x)      const;
        double camberSlope(double x) const;
        double length()              const;
        double midLineAngle()        const;
        double maxCamber()           const;
        double maxThickness()        const;
        double thickness(double x)   const;
        double topSlope(double x)    const;
        double xCamber()             const;
        double xThickness()          const;

        void setCamber(double xcamb, double camb);
        void setThickness(double xthick, double thick);
        void setThickness(std::vector<double> const &thicknesses) {m_Thickness=thicknesses;}

        void makeBaseFromCamberAndThickness();
        void rebuildPointSequenceFromBase();

        bool rePanel(int NPanels, double amplitude);
        double normalizeGeometry();
        double deRotate();

        bool makeTopBotSurfaces();

        void makeCubicSpline(int nCtrlPts=-1);
        void makeCubicSpline(CubicSpline &cs, int nCtrlPts=-1) const;
        void makeNormalsFromCubic();

        bool makeApproxBSpline(BSpline &bs, int deg, int nCtrlPts, int nOutputPts) const;

        bool sharpTE() const;
        Vector2d TEbisector() const;
        Vector2d const &TE() const {return m_TE;}
        Vector2d const &LE() const {return m_LE;}
        void setLE(Vector2d const &le) {m_LE=le;}
        void setTE(Vector2d const &te) {m_TE=te;}

        bool exportFoilToDat(std::string &out) const;
        bool initGeometry();

        void appendBasePoint(double x, double y);

        void copy(Foil const *pSrcFoil, bool bMeta=true);
        void copy(Foil const &SrcFoil, bool bMeta=true, bool bForceDeepCopy=false);

        void setFlaps();
        void setLEFlapData(bool bFlap, double xhinge, double yhinge, double angle);
        void setTEFlapData(bool bFlap, double xhinge, double yhinge, double angle);
        void setTEFlapAngle(double angle){m_TEFlapAngle=angle;}


        bool hasTEFlap() const {return m_bTEFlap;}
        bool hasLEFlap() const {return m_bLEFlap;}
        void setTEFlap(bool b) {m_bTEFlap=b;}
        void setLEFlap(bool b) {m_bLEFlap=b;}

        double TEFlapAngle() const {return m_TEFlapAngle;}
        double LEFlapAngle() const {return m_LEFlapAngle;}

        void setTEHinge(double fracchord, double fracheight) {m_TEXHinge=fracchord; m_TEYHinge=fracheight;}
        void setTEXHinge(double fracchord) {m_TEXHinge=fracchord;}
        void setTEYHinge(double fracheight) {m_TEYHinge=fracheight;}
        double TEXHinge() const {return m_TEXHinge;}
        double TEYHinge() const {return m_TEYHinge;}
        Vector2d TEHinge() const;

        double LEXHinge() const {return m_LEXHinge;}
        double LEYHinge() const {return m_LEYHinge;}
        Vector2d LEHinge() const;

        void scaleHingeLocations(); // cleaning old files

        bool serializeXfl(QDataStream &ar, bool bIsStoring);
        bool serializeFl5(QDataStream &ar, bool bIsStoring);

        void interpolate(Foil const *pFoil1, Foil const *pFoil2, double frac);

        bool isCamberLineVisible() const {return m_bCamberLine;}
        void showCamberLine(bool bVisible) {m_bCamberLine = bVisible;}

        const std::string & description() const {return m_Description;}
        void setDescription(std::string const &desc) {m_Description=desc;}

        std::string properties(bool bLong=false) const;

        int nNodes()  const {return int(m_Node.size());}
        int nBaseNodes() const {return int(m_BaseNode.size());}

        double x(int k) const {return m_Node.at(k).x;}
        double y(int k) const {return m_Node.at(k).y;}
        double xb(int k) const {return m_BaseNode.at(k).x;}
        double yb(int k) const {return m_BaseNode.at(k).y;}

        void clearPointArrays();
        void resizePointArrays(int size);

        void applyBase();

        int iSelectedPt() const {return m_iSelectedPt;}
        void setSelectedPt(int iH) {m_iSelectedPt = iH;}

        bool isFilled() const {return m_bFill;}
        void setFilled(bool bFill) {m_bFill=bFill;}

        std::string listCoords(bool bBaseCoords=false);
        void listSurface(bool bBase);

        std::vector<Node2d> const & baseCbLine() const {return m_BaseCbLine;}
        std::vector<Node2d> const & CbLine() const {return m_CbLine;}

        std::vector<double> const &thickness() const {return m_Thickness;}

        void setBaseCamberLine(std::vector<Node2d> const &cb) {m_BaseCbLine=cb;}
        void setThicknessLine(std::vector<double> const &th) {m_Thickness=th;}

//        void setBaseCamberLine(std::vector<Node2d> const &cb);
//        void setThicknessLine(std::vector<double> const &th);

        void resizeBaseArrays(int size) {m_BaseTop.resize(size); m_BaseBot.resize(size);}

        void setBaseNodes(std::vector<Node2d> const &Nodes) {m_BaseNode=Nodes;}
//        void setBaseNodes(std::vector<Node2d> const &Nodes);
        void setBaseNode(int i, double x, double y) {if(i>=0&&i<int(m_BaseNode.size())) m_BaseNode[i].set(x,y);}

        void setBaseTop(std::vector<Node2d> const &nodes) {m_BaseTop=nodes;}
        void setBaseBot(std::vector<Node2d> const &nodes) {m_BaseBot=nodes;}

//        void setBaseTop(std::vector<Node2d> const &nodes);
//        void setBaseBot(std::vector<Node2d> const &nodes);

        int LEindex() const {return m_iLE;}

        CubicSpline const & cubicSpline() const {return m_CubicSpline;}

        void makeModPermanent();

        double CSfracLE() const {return m_CSfracLE;}

        void setBunchParameters(Spline::enumBunch bunchtype, double bunchamp) {m_BunchType=bunchtype, m_BunchAmp=bunchamp;}


        void setNodes(std::vector<Node2d> const& nodes) {m_Node=nodes;}
        Node2d const &frontNode() const {return m_Node.front();}
        Node2d const &backNode() const {return m_Node.back();}


    private:
        void resetFoil();

        void setLEFromCubicSpline();

        void setTEFlap();
        void setLEFlap();


    private:

        std::string m_Description;	         /**< a free description */

        int m_iSelectedPt;                    /**< the index of the point to highlight in the display */
        bool m_bFill;                        /**< true if the inside of the foil should be displayed with a solid color */

        bool m_bCamberLine;                  /**< true if the foil mid camber line is to be displayed */

        CubicSpline m_CubicSpline;

        // depending on how the foil was constructed, base Top and Bot arrays can be created first
        // and then base Nodes are constructed, or the other way round
        std::vector<Node2d> m_BaseNode;          /**< the array of base foil points, i.e. non-flapped, not repanelled */
        std::vector<Node2d> m_BaseTop;	     /**< the upper surface points of the base geometry, i.e. non-flapped, not repanelled */
        std::vector<Node2d> m_BaseBot;         /**< the lower surface points of the base geometry, i.e. non-flapped, not repanelled */

        std::vector<Node2d> m_Node;              /**< the array of coordinates of the flapped repanelled foil*/

        std::vector<Node2d> m_Top;	         /**< the upper surface points = base + flapped + repanelled */
        std::vector<Node2d> m_Bot;	         /**< the lower surface points = base + flapped + repanelled */

        int m_iLE;                           /**< the index of the leading edge point - defined as the point of min x. */

        Vector2d m_TE;                       /**< the trailing edge point */
        Vector2d m_LE;                       /**< the leading edge point. Note: NOT a foil point*/

        double m_CSfracLE;         /**< the cubic spline parameter such that m_CSpline(frac)=LE */

        std::vector<Node2d> m_CbLine;          /**< the mid camber line points for the flapped foil */


        bool m_bTEFlap;          /**< true if the foil has a trailing edge flap */
        bool m_bLEFlap;          /**< true if the foil has a leading edge flap */
        double m_TEFlapAngle;    /**< the trailing edge flap angle, in degrees*/
        double m_TEXHinge;       /**< the x-position of the trailing edge flap, as a fraction of the chord length */
        double m_TEYHinge;       /**< the y-position of the trailng edge flap, as a fraction of the chord length*/

        double m_LEFlapAngle;    /**< the leading edge flap angle, in degrees */
        double m_LEXHinge;       /**< the x-position of the leading edge flap, as a fraction of the chord length */
        double m_LEYHinge;       /**< the y-position of the leading edge flap, as a fraction of the chord length */

        std::vector<Node2d> m_BaseCbLine;      /**< the mid camber line points for the base unflapped foil */
        std::vector<double> m_Thickness;         /**< the local thicknesses at each mid point*/


        double m_BunchAmp;  /** k=0.0 --> uniform bunching, k=1-->full varying bunch */
        Spline::enumBunch m_BunchType;
};



