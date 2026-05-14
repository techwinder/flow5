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
#include <QCheckBox>
#include <QComboBox>
#include <QSettings>

#include <api/triangle3d.h>

class PlainTextOutput;
class FloatEdit;

class StlReaderDlg : public QDialog
{
    Q_OBJECT

    public:
        StlReaderDlg(QWidget *pParent);
        QString logMsg() const;

        std::vector<Triangle3d> const & triangleList() const {return m_Triangle;}

        bool importTrianglesFromMeshFile(const QString &FileName, double unitfactor);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(750,500);}


    signals:
        void outputMsg(QString);

    private slots:
        void onImportMeshFile();
        void onButton(QAbstractButton *pButton);


    private:
        QDialogButtonBox *m_pButtonBox;
        QComboBox *m_pcbLengthUnitSel;
        PlainTextOutput *m_ppto;
        QPushButton *m_ppbImport;

        std::vector<Triangle3d> m_Triangle;


        static QByteArray s_Geometry;
        static int s_LengthUnitIndex;
};

