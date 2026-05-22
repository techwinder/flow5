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



#include <externalsail.h>
#include <occmeshparams.h>
#include <triangle3d.h>

class FL5LIB_EXPORT SailOcc : public ExternalSail
{    
    friend class  SailDlg;
    friend class  OccSailDlg;

    public:
        SailOcc();

        bool isOccSail()    const override {return true;}
        bool isStlSail()    const override {return false;}

        Sail* clone() const override {return new SailOcc(*this);}

        Vector3d point(double , double , xfl::enumSurfacePosition) const override {return Vector3d();}
        void duplicate(Sail const*pSail) override;
        void flipXZ() override;
        void makeDefaultSail() override {}
        void makeSurface() override;
        void makeTriangulation(int nx=s_iXRes, int nz=s_iZRes) override;
        void properties(std::string &properties, std::string const &prefx, bool bFull=false) const override;
        void resizeSections(int , int ) override {}
        void rotate(const Vector3d &origin, Vector3d const &axis, double theta) override;
        void scale(double XFactor, double YFactor, double ZFactor) override;
        void scaleAR(double newAR) override;
        void translate(Vector3d const &T) override;
        void shapesToBreps();

        NCollection_List<TopoDS_Shape> &shapes() {return m_Shape;}
        NCollection_List<TopoDS_Shape> const &shapes() const {return m_Shape;}
        void clearShapes() {m_Shape.Clear();}
        void appendShape(TopoDS_Shape const &shape) {m_Shape.Append(shape);}

        OccMeshParams &occTessParams() {return m_OccTessParams;}
        OccMeshParams const &occTessParams() const {return m_OccTessParams;}
        void setOccTessParams(OccMeshParams const &params) {m_OccTessParams=params;}


        std::vector<std::string> const &bReps() const {return m_BRep;}
        std::vector<std::string> &bReps() {return m_BRep;}

        std::string const &logMsg() const {return m_LogMsg;}
        void clearLogMsg() {m_LogMsg.clear();}

    private:
        NCollection_List<TopoDS_Shape> m_Shape;  /** The list of shapes of which this sail is made. */

        std::vector<std::string> m_BRep;

        OccMeshParams m_OccTessParams; /** @todo move into Part class with serialization format change */




        mutable std::string m_LogMsg;

};


