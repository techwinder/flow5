/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflobjects/objects3d/analysis/wpolar.h>


class WPolarExt : public WPolar
{
    public:
        WPolarExt();

        double getVariable(int iVariable, int index) const override;
        void setData(int iVariable, int index, double value) override;
        int dataSize() const override;
        void resizeData(int newsize) override;
        bool serializeFl5v726(QDataStream &ar, bool bIsStoring) override;
        bool serializeFl5v750(QDataStream &ar, bool bIsStoring) override;
        void clearData();
        void copy(WPolar const *pWPolar) override;
        bool hasPoints() const override {return dataSize()>0;}
        void insertDataPointAt(int index, bool bAfter) override;
        void removeAt(int index) override;

    private:
        QVector<QVector<double>> m_data;
};

