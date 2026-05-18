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
#include <QRadioButton>
#include <QLabel>
#include <QListWidget>
#include <QComboBox>
#include <QSettings>



class PlaneXfl;
class WingXfl;
class FuseXfl;
class Fuse;
class Sail;
class IntEdit;
class PlainTextOutput;
class Triangle3d;

class STLWriterDlg : public QDialog
{
    Q_OBJECT

    public:
        STLWriterDlg(QWidget *pParent);
        void initDialog(PlaneXfl *pPlane, WingXfl *pWing, Fuse *pFuse, Sail *pSail);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private:
        void setupLayout();
        void readParams();

        void makeSTLTriangulation(WingXfl const *pWingXfl, std::vector<Triangle3d> &triangles, int CHORDPANELS, int SPANPANELS, double scalefactor=1.0) const;


        QSize sizeHint() const override {return QSize(700,700);}

        void accept() override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private slots:
        void onSetLabels();
        void onExporttoSTL();

    private:
        PlaneXfl const *m_pPlane;
        WingXfl const *m_pWing;
        Fuse const *m_pFuse;
        Sail *m_pSail;
        double m_UnitFactor;


        IntEdit *m_pieChordPanels, *m_pieSpanPanels;
        QRadioButton *m_prbBinary, *m_prbASCII;
        QListWidget *m_plwNameList;
        QLabel *m_plabChord, *m_plabSpan;
        PlainTextOutput *m_ppto;
        QComboBox *m_pcbLengthUnitSel;
        QStringList m_SelectedList;


        static int s_LengthUnitIndex;
        static QByteArray s_Geometry;

        static bool s_bBinary;
        static int s_iChordPanels;
        static int s_iSpanPanels;
};


