/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_ListOfShape.hxx>

#include <xflobjects/sailobjects/sails/externalsail.h>
#include <xflocc/occmeshparams.h>

#include <xflgeom/geom3d/triangle3d.h>

class OccSail : public ExternalSail
{    
    friend class SailDlg;
    friend class OccSailDlg;

    public:
        OccSail();

        void setTriangles(QVector<Triangle3d> const &triangles);

        bool isOccSail()    const override {return true;}
        bool isStlSail()    const override {return false;}

        Sail* clone() const override {return new OccSail(*this);}

        Vector3d point(double , double , xfl::enumSurfacePosition) const override {return Vector3d();}
        bool serializeSailFl5(QDataStream &ar, bool bIsStoring) override;
        void duplicate(Sail const*pSail) override;
        void flipXZ() override;
        void makeDefaultSail() override {}
        void makeSurface() override {}
        void makeTriangulation(int nx=s_iXRes, int nz=s_iZRes) override;
        void properties(QString &props, QString const &prefix, bool bFull=false) const override;
        void resizeSections(int , int ) override {}
        void rotate(const Vector3d &origin, Vector3d const &axis, double theta) override;
        void scale(double XFactor, double YFactor, double ZFactor) override;
        void scaleAR(double newAR) override;
        void translate(Vector3d const &T) override;


        TopoDS_ListOfShape &shapes() {return m_Shape;}
        TopoDS_ListOfShape const &shapes() const {return m_Shape;}
        void clearShapes() {m_Shape.Clear();}
        void appendShape(TopoDS_Shape const &shape) {m_Shape.Append(shape);}

        OccMeshParams const &occMeshParams() const {return m_OccMeshParams;}
        void setOccMeshParams(OccMeshParams const &params) {m_OccMeshParams=params;}

        QString const &logMsg() const {return m_LogMsg;}
        void clearLogMsg() {m_LogMsg.clear();}

    private:
        TopoDS_ListOfShape m_Shape;  /** The list of shapes of which this sail is made. */


        OccMeshParams m_OccMeshParams;

        mutable QString m_LogMsg;

};


