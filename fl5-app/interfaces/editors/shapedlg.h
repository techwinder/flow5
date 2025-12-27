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

#include <QSettings>


#include <TopoDS_Shape.hxx>


#include <QDialog>
#include <QDialogButtonBox>
#include <QSplitter>
#include <QListWidget>

#include <api/quaternion.h>

class gl3dShapesView;
class gl3dGeomControls;
class PlainTextOutput;

class ShapeDlg : public QDialog
{
    Q_OBJECT

    public:
        ShapeDlg(QWidget *pParent);

        void initDialog(const TopoDS_ListOfShape &shapes);

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *) override;
        QSize sizeHint() const override {return QSize(900,700);}

        QVector<TopoDS_Shape> const &shapes() const {return m_Shapes;}
        TopoDS_Shape const &shape(int ish) {return m_Shapes.at(ish);}
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onButton(QAbstractButton *pButton);
        void onShellChanged();
        void onDeleteShell();

    private:
        void setupLayout();
        void connectSignals();
        void fillShapes(const TopoDS_ListOfShape &shapes);

    private:

        QVector<TopoDS_Shape> m_Shapes;

        gl3dShapesView *m_pglShapesView;
        gl3dGeomControls *m_pglControls;

        QSplitter *m_pHSplitter, *m_pVSplitter;
        QListWidget *m_plwShapes;
        QPushButton *m_ppbDelete;
        PlainTextOutput *m_pptoOutput;
        QDialogButtonBox *m_pButtonBox;

        static Quaternion s_ab_quat;
        static QByteArray s_Geometry;
        static QByteArray s_HSplitterSizes, s_VSplitterSizes;
};

