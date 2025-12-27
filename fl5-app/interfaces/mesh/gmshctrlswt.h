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


#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

#include <api/gmshparams.h>


class FloatEdit;
class IntEdit;
struct GmshParams;

class GmshCtrlsWt : public QWidget
{
    Q_OBJECT
    public:
        GmshCtrlsWt(QWidget *parent = nullptr);

        void initWt(const GmshParams &params);
        void updateUnits();

        int nCurvature() const;
        void setnCurvature(int n);

        GmshParams params() const;

        bool isChanged() const {return m_bChanged;}

    private:
        void setupLayout();
        void connectSignals();
        void setControls();

    private slots:
        void onParamChanged();


    private:

        FloatEdit *m_pfeMinSize, *m_pfeMaxSize;
        IntEdit *m_pienCurvature;


        bool m_bChanged;

};

