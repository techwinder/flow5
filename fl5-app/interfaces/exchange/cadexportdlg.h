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
#include <QSettings>
#include <QListWidget>
#include <QRadioButton>
#include <QLabel>
#include <QString>
#include <QComboBox>
#include <QSplitter>
#include <QDialogButtonBox>

#include <TopoDS_ListOfShape.hxx>
#include <TopoDS_Shape.hxx>

class PlainTextOutput;

class CADExportDlg : public QDialog
{
    Q_OBJECT

    public:
        CADExportDlg(QWidget*pParent);
        void init(TopoDS_Shape const & shape, QString partname);
        void init(TopoDS_ListOfShape const & listofshape, QString partname);

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        QSize sizeHint() const override {return QSize(700, 500);}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    protected:
        void exportBRep();
        void exportSTEP();

        void updateStdOutput(std::string const &strong);
        void updateOutput(QString const &strong);

        void makeCommonWts();
        void setupLayout();
        virtual void exportShapes();

    protected slots:
        void onExport();
        void onFormat();
        void onButton(QAbstractButton *pButton);


    protected:
        QListWidget *m_plwListFormat;
        QRadioButton *m_prbBRep, *m_prbSTEP;
        QPushButton *m_ppbExport;
        QDialogButtonBox *m_pButtonBox;
        PlainTextOutput *m_ppto;

        QFrame *m_pfrControls;

        TopoDS_ListOfShape m_ShapesToExport;

        QString m_PartName;


        static int s_ExportIndex;
        static QByteArray s_Geometry;
};

