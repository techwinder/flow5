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

#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QString>
#include <QLineEdit>
#include <QCheckBox>
#include <QSettings>

#include <api/enums_objects.h>
#include <interfaces/widgets/customdlg/xfldialog.h>

class Foil;
class Polar;
class FloatEdit;

class FoilPolarDlg : public XflDialog
{
    Q_OBJECT

    public:
        FoilPolarDlg(QWidget *pParent=nullptr);

        void initDialog(const Foil *pFoil, Polar *pPolar=nullptr);
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


        static Polar & thePolar() {return s_Polar;}


    private:
        void readData();
        void setupLayout();
        void connectSignals();
        void enableControls();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;


        void setAutoName(bool bAutoName){m_bAutoName=bAutoName;}

    private slots:

        void onPolarType();
        void onEditingFinished();
        void onAutoName(bool bChecked);
        void onNameOptions();
        void onButton(QAbstractButton *pButton) override;
        void setPlrName();

    private:
        QLabel *m_plabFoilName;
        QRadioButton *m_prbType1, *m_prbType2, *m_prbType3, *m_prbType4, *m_prbType6;

        QLabel *m_plabRe;
        QLabel *m_plabMach;

        FloatEdit *m_pfeAlpha;
        FloatEdit *m_pfeReynolds;
        FloatEdit *m_pfeMach;

        QLineEdit *m_pleAnalysisName;
        QCheckBox *m_pchAutoName;

        FloatEdit *m_pfeNCrit, *m_pfeTopTrans, *m_pfeBotTrans;

        QLabel *m_plabHinge;
        FloatEdit *m_pfeTheta;

        Foil const* m_pFoil;
        bool  m_bAutoName;

        int m_MaTypDef, m_ReTypDef;


        static QByteArray s_WindowGeometry;
        static Polar s_Polar;
};



