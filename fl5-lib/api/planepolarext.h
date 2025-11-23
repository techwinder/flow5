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

#include <planepolar.h>


class FL5LIB_EXPORT PlanePolarExt : public PlanePolar
{
    public:
        PlanePolarExt();

        double getVariable(int iVariable, int index) const override;
        void setData(int iVariable, int index, double value) override;
        int dataSize() const override;
        void resizeData(int newsize) override;
        bool serializeFl5v726(QDataStream &ar, bool bIsStoring) override;
        bool serializeFl5v750(QDataStream &ar, bool bIsStoring) override;
        void clearData();
        void copy(PlanePolar const *pWPolar) override;
        bool hasPoints() const override {return dataSize()>0;}
        void insertDataPointAt(int index, bool bAfter) override;
        void removeAt(int index) override;

    private:
        std::vector<std::vector<double>> m_data;
};

