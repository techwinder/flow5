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

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>

#include <core/fontstruct.h>
class Foil;
class Plane;
class Boat;
class Polar;
class Polar3d;

class ExpandableTreeView;
class ObjectTreeDelegate;
class ObjectTreeModel;

class AnalysisSelDlg : public QDialog
{
    Q_OBJECT
    public:
        enum enumObject {FOIL, PLANE, BOAT};

    public:
        AnalysisSelDlg(QWidget *parent = nullptr);
        void initDialog(const Foil *pCurFoil, const Plane *pCurPlane, const Boat *pCurBoat);

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void accept() override;
        QSize sizeHint() const override {return QSize(450, 550);}

        QVector<Polar const*> selected2dPolars() {return m_Selected2dPolars;}
        Polar const* selected2dPolar(int ip) {if(ip<0||ip>=m_Selected2dPolars.size()) return nullptr; else return m_Selected2dPolars.at(ip);}

        QVector<Polar3d const*> selected3dPolars() {return m_Selected3dPolars;}
        Polar3d const* selectedPolar3d(int ip) {if(ip<0||ip>=m_Selected3dPolars.size()) return nullptr; else return m_Selected3dPolars.at(ip);}

    private:
        void setupLayout();

    private slots:
        void onButton(QAbstractButton *pButton);

    private:
        QDialogButtonBox *m_pButtonBox;

        QLabel *m_plabTitle;

        ExpandableTreeView *m_pStruct;
        ObjectTreeModel *m_pModel;
        ObjectTreeDelegate *m_pDelegate;

        QVector<Polar const*> m_Selected2dPolars;
        QVector<Polar3d const*> m_Selected3dPolars;

        enumObject m_Object;

        static QByteArray s_Geometry;

};


