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


#include <xflmesh.h>

#include <panel4.h>


class Node;
class Polar3d;

class FL5LIB_EXPORT QuadMesh : public XflMesh
{
    public:
        QuadMesh();
        virtual ~QuadMesh() = default;

        void clearMesh()   override {m_Panel4.clear(); m_Node.clear(); m_WakePanel4.clear();}
        void clearPanels() override {m_Panel4.clear(); m_WakePanel4.clear();}
        int nPanels() const override {return int(m_Panel4.size());}
        int nWakePanels() const override {return int(m_WakePanel4.size());}
        int panelCount() const override {return int(m_Panel4.size());}
        int nWakeColumns() const override;

        void meshInfo(std::string &info) const override;
        void cleanMesh(std::string &logmsg) override;
        void getMeshInfo(std::string &logmsg) const override;
        void getLastTrailingPoint(Vector3d &pt) const override;

        void rotate(double alpha, double beta, double phi) override;


        void getFreeEdges(std::vector<Segment3d> &freeedges, std::vector<QPair<int, int> > &pairerrors) const;

        void clearWakePanels() {m_WakePanel4.clear();}

        void checkPanels(std::string &log, bool bMinAngle, bool bMinArea, bool bWarp, std::vector<int> &minanglelist, std::vector<int> &minarealist, std::vector<int> &warplist, double minangle, double minarea, double maxquadwarp);

        Panel4 &panel(int index) {return m_Panel4[index];}
        Panel4 const &panelAt(int index) const {return m_Panel4.at(index);}
        Panel4 &lastPanel() {return m_Panel4.back();}
        Panel4 const &lastPanel() const {return m_Panel4.back();}

        std::vector<Panel4> & panels() {return m_Panel4;}
        std::vector<Panel4> const & panels() const {return m_Panel4;}
        void addPanel(Panel4 const &p4) {m_Panel4.push_back(p4);}
        void addPanels(std::vector<Panel4>const &Panel4list);

        std::vector<Panel4> & wakePanels() {return m_WakePanel4;}
        std::vector<Panel4> const & wakePanels() const {return m_WakePanel4;}
        Panel4 const &wakePanel(int index) const {return m_WakePanel4.at(index);}

        void removeLastPanel() {m_Panel4.pop_back();}

        void makeWakePanels(double nxWakePanels, double wakepanelfactor, double TotalWakeLength, Vector3d const &WindDir, bool bAlignWakeTE);

        static int makeWakePanels(std::vector<Panel4> &m_Panel4, double nxWakePanels, double wakepanelfactor, double TotalWakeLength, Vector3d const &WindDir,
                                  std::vector<Panel4>&WakePanel4, std::vector<Vector3d> &WakeNode, int &nWakeColumn, bool bAlignWakeTE);

    private:
        std::vector<Panel4> m_Panel4;
        std::vector<Panel4> m_WakePanel4;

};


