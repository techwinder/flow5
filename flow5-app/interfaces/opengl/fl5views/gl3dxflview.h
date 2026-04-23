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

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_ListOfShape.hxx>

#include <QTimer>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QLabel>

#include <api/enums_objects.h>
#include <interfaces/opengl/controls/arcball.h>
#include <interfaces/opengl/views/gl3dview.h>

#include <api/vector2d.h>
#include <api/segment3d.h>

#define MAXCPCOLORS    21

class Boat;
class Sail;
class Fuse;
class FuseOcc;
class FuseXfl;
class MainFrame;
class NURBSSurface;
class Panel4;
class Part;
class Plane;
class PlaneXfl;
class PlaneOpp;
class PointMass;
class Triangle3d;
class PlanePolar;
class WingXfl;
class WingOpp;
class XPlane;
class glPartBuffers;
class Panel3;
class AngleControl;


namespace xfl
{
    /** @enum The different objects that can be picked, but only one at a time*/
    enum enumPickObjectType {VERTEX, MESHNODE, TRIANGLENODE, PANEL3, PANEL4, TRIANGLE3D, SEGMENT3D, FACE, NOPICK};
}


class gl3dXflView : public gl3dView
{
    Q_OBJECT
    public:
        gl3dXflView(QWidget *pParent = nullptr);
        virtual ~gl3dXflView()  override;

    public slots:

        virtual void onSurfaces(bool bChecked);
        virtual void onPanels(bool bChecked);
        virtual void onNormals(bool bChecked);
        virtual void onOutline(bool bChecked);
        void onFoilNames(bool bChecked);
        void onShowMasses(bool bChecked);
        void onTessellation(bool bChecked);
        void onHighlightPanel(bool bChecked);
        void onCtrlPoints(bool bChecked);
        void onCornerPts(bool bChecked);

        virtual void onPartSelClicked() {}

    public:
        void setLabelFonts();

        void resetgl3dMesh()  {m_bResetglMesh = true;}

        void glDrawMasses(const Plane *pPlane);

        void paintFlaps(PlaneXfl const* pPlaneXfl, const PlanePolar *pWPolar, PlaneOpp const *pPOpp);
        void paintFoilNames(const WingXfl *pWing);
        void paintPartMasses(const Vector3d &pos, double volumeMass, const QString &tag, const std::vector<PointMass> &ptMasses, int iSelectedMass);

        void paintTriPanels(QOpenGLBuffer &vbo, bool bFrontAndBack);

        void paintNormals(QOpenGLBuffer &vbo);


        void paintStreamLines(QOpenGLBuffer &vbo, const QVector<QColor> &linecolors, int NX);

        void paintSailCornerPoints(Sail const*pSail, const Vector3d &pos);

        void setSelectedPlaneMass(int iMass) {m_iSelectedPlaneMass=iMass;}
        void setSelectedPartMass(int iMass) {m_iSelectedPartMass=iMass;}
        void setPlaneReferenceLength(const Plane *pPlane);
        void setBoatReferenceLength(const Boat *pBoat);

        void updatePartFrame(const Plane *pPlane);
        void updatePartFrame(Boat const *pBoat);

        void setFlags(bool bOutline, bool bSurfaces, bool bVLMPanels, bool bAxes, bool bMasses, bool bFoilNames, bool bTessellation, bool bNormals, bool bCtrlPoints);
        virtual bool bOutline()         const {return m_bOutline;}
        virtual bool bSurfaces()        const {return m_bSurfaces;}
        virtual bool bVLMPanels()       const {return m_bMeshPanels;}
        virtual bool bNormals()         const {return m_bNormals;}
        virtual bool bTriangleOutline() const {return m_bTessellation;}
        virtual bool bCtrlPts()         const {return m_bCtrlPoints;}
        virtual bool bCornerPts()       const {return m_bSailCornerPts;}

        bool bHigh()      const {return m_bHighlightPanel;}
        bool bFoilNames() const {return m_bFoilNames;}
        bool bMasses()    const {return m_bShowMasses;}

        virtual void showOutline(bool boutline)    {m_bOutline=boutline;}
        virtual void showSurfaces(bool bsurfaces)  {m_bSurfaces=bsurfaces;}
        virtual void showPanels(bool bpanels)      {m_bMeshPanels=bpanels;}
        virtual void showNormals(bool bnormals)    {m_bNormals=bnormals;}
        virtual void showTriangleOutline(bool bTriOutline)   {m_bTessellation=bTriOutline;}
        virtual void showControlPoints(bool bShow) {m_bCtrlPoints=bShow;}
        void showCornerPoints(bool bShow)  {m_bSailCornerPts=bShow;}
        void showMasses(bool bmasses)      {m_bShowMasses=bmasses;}
        void showFoilNames(bool bfoils)        {m_bFoilNames=bfoils;}
        void setHighlighting(bool bHigh)   {m_bHighlightPanel=bHigh;}

        bool bSailCornerPts() {return m_bSailCornerPts;}

        void clearSelectedParts() {m_SelectedParts.clear();}
        void setSelectedPart(int index);
        bool hasSelectedPart(){return m_SelectedParts.count()>0;}


        void setTopRightOutput(std::string const &info) {setTopRightOutput(QString::fromStdString(info));}
        void setTopRightOutput(QString const &info);
        void clearTopRightOutput() {m_plabTopRight->clear();}

        void setBotRightOutput(std::string const &info) {setBotRightOutput(QString::fromStdString(info));}
        void setBotRightOutput(QString const &info);
        void clearBotRightOutput() {m_plabBotRight->clear();}

        void setBotLeftOutput(std::string const &info) {setBotLeftOutput(QString::fromStdString(info));}
        void setBotLeftOutput(QString const &info);
        void clearBotLeftOutput() {m_plabBotLeft->clear();}


        void selectPanels(bool bSelect) {m_bP3Select=bSelect;}

        bool isPicking() const {return m_PickType!=xfl::NOPICK;}
        void setPicking(xfl::enumPickObjectType type) {m_PickType=type;}
        xfl::enumPickObjectType pickType() const {return m_PickType;}
        bool bPickNode()   const {return m_PickType==xfl::TRIANGLENODE||m_PickType==xfl::MESHNODE;}
        bool bPickVertex() const {return m_PickType==xfl::VERTEX;}
        bool bPickPanel3() const {return m_PickType==xfl::PANEL3;}
        bool bPickPanel4() const {return m_PickType==xfl::PANEL4;}
        bool bPickFace()   const {return m_PickType==xfl::FACE;}
        void stopPicking() {m_PickType = xfl::NOPICK;}

        virtual void resetPickedNodes() {m_NodePair={-1,-1}; m_PickedNodeIndex=-1;}

        void clearHighlightList() {m_PanelHightlight.clear();}
        void appendHighlightList(QVector<int> const &list) {m_PanelHightlight.append(list);}
        void setHighlightList(QVector<int> const &list) {m_PanelHightlight=list;}

        void showPartFrame(bool bShow) {if(m_pfrParts) m_pfrParts->setVisible(bShow);}

        void setNodes(std::vector<Node> const &nodes) {m_Nodes.clear(); for(Node const &nd : nodes) m_Nodes.push_back(nd);}
        void setNodes(QVector<Node> const &nodes) {m_Nodes=nodes;}
        void appendNodes(QVector<Node> const &nodes) {m_Nodes.append(nodes);}
        QVector<Node> & Nodes() {return m_Nodes;}
        void clearNodes() {m_Nodes.clear();}
        void showEdgeNodes(bool bShow) {m_bEdgeNodes=bShow;}

        void setSegments(QVector<Segment3d> const &segs) {m_Segments=segs; m_bResetglSegments=true;}
        QVector<Segment3d> & segments() {return m_Segments;}
        void clearSegments() {m_Segments.clear(); m_bResetglSegments=true;}
        void resetglSegments() {m_bResetglSegments=true;}

        void setSurfacePick(xfl::enumSurfacePosition pos) {m_SurfacePick=pos;}

        void setMeasure(Segment3d const &seg) {m_Measure=seg;}
        void clearMeasure() {m_Measure.reset(); m_PickType=xfl::NOPICK;}

        virtual double objectReferenceLength() const {return 1.0;}


        static double velocityScale() {return s_VelocityScale;}
        static void setVelocityScale(double scale) {s_VelocityScale=scale;}


    protected:

        void resizeLabels();

        void pickTriangleNode(Triangle3d const &p3, Vector3d const &I);
        void pickPanelNode(const Panel3 &p3, const Vector3d &I, xfl::enumSurfacePosition pos);
        void pickNode(QPoint const &point, QVector<Node> const &nodes, xfl::enumSurfacePosition pos);
        int pickFace(const QPoint &point, TopoDS_Shape const &shape, TopoDS_Face &pickedface);
        QPair<int, int> pickFace(const QPoint &point, TopoDS_ListOfShape const &shapes);

        virtual void highlightPickedPanel3(const Panel3 &p3);
        virtual bool pickPanel3(QPoint const &point, const std::vector<Panel3> &panels, Vector3d &I);
        bool pickTriangle3d(QPoint const &point, const std::vector<Triangle3d> &panels, Vector3d &I);

        bool pickShapeVertex(QPoint const &point, TopoDS_ListOfShape const&shapes, Vector3d &I);

        void paintMeasure();

        void resizeGL(int w, int h) override;
        void showEvent(QShowEvent *pEvent) override;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        void enterEvent(QEvent *pEvent) override;
#else
        void enterEvent(QEnterEvent *pEvent) override;
#endif
        void leaveEvent(QEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;


    signals:
        void pickedNode(Vector3d);
        void pickedNodeIndex(int);
        void pickedNodePair(QPair<int,int>);
        void pickedEdge(int, int);
        void pickedFace(int, int);
        void panelSelected(int);
        void partSelected(Part *);

    protected:

        bool m_bOutline;                   /**< true if the surface outlines are to be displayed in the 3D view*/
        bool m_bSurfaces;                  /**< true if the surfaces are to be displayed in the 3D view*/
        bool m_bMeshPanels;                 /**< true if the panels are to be displayed in the 3D view*/
        bool m_bShowMasses;                /**< true if the point masses are to be displayed on the openGL 3D view */
        bool m_bFoilNames;                 /**< true if the foil names are to be displayed on the openGL 3D view */
        bool m_bNormals;
        bool m_bHighlightPanel;
        bool m_bTessellation;
        bool m_bCtrlPoints;
        bool m_bSailCornerPts;

        bool m_bResetglSegments;
        bool m_bResetglMesh;

        xfl::enumPickObjectType m_PickType;

        int m_iSelectedPlaneMass;
        int m_iSelectedPartMass;

        bool m_bP3Select;

        int m_iMomentPoints;

        Part const *m_pSelectedPart;
        QVector<int> m_SelectedParts;

        QVector<Vector3d> m_VelocityPt;
        QVector<Vector3d> m_VelocityVec;

        QPair<int, int> m_NodePair;
        int m_PickedPanelIndex;
        Vector3d m_PickedPoint;
        int m_PickedNodeIndex;
        bool m_bPickedVertex;

        int m_HighFace, m_HighEdge;

        QVector<int> m_PanelHightlight;
        QOpenGLBuffer m_vboHighlightPanel3;
        QOpenGLBuffer m_vboTriangle;

        bool m_bEdgeNodes;
        QVector<Node> m_Nodes;
        QVector<Segment3d> m_Segments;
        QOpenGLBuffer m_vboSegments; // free edge highlighting

        Segment3d m_Measure;

        QLabel *m_plabBotLeft;
        QLabel *m_plabBotRight;
        QLabel *m_plabTopRight;

        QFrame *m_pfrParts;

        xfl::enumSurfacePosition m_SurfacePick;


        static float s_NodeRadius;

        static bool s_bCirculation;
        static bool s_bVortonLine;
        static bool s_bNegVortices;

        static double s_VelocityScale;
        static bool s_bResetglGridVelocities;
        static bool s_bResetglVorticity;
};

