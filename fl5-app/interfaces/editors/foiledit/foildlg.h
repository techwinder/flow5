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
#include <QFrame>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QPoint>
#include <QSize>
#include <QSettings>

#include <core/fontstruct.h>
#include <interfaces/widgets/view/grid.h>
#include <interfaces/widgets/customdlg/xfldialog.h>

class Foil;
class FoilWt;


class FoilDlg : public XflDialog
{
    Q_OBJECT

    public:
        FoilDlg(QWidget *pParent);
        virtual ~FoilDlg();
        virtual void initDialog(Foil *pFoil=nullptr);
        virtual void hideEvent(QHideEvent *pEvent) override;
        virtual void keyPressEvent(QKeyEvent *pEvent) override;

        virtual void resizeEvent(QResizeEvent *) override;

        QSize sizeHint() const override;

        void resetFoil();
        Foil const *bufferFoil() const {return m_pBufferFoil;}


        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static void setGrid(Grid const &g) {s_Grid=g;}

    protected slots:
        virtual void onOK();
        virtual void onChanged();
        virtual void onReset() override;
        virtual void reject() override;
        void onButton(QAbstractButton*pButton)  override;

    protected:
        void makeCommonWidgets();
        virtual void readParams() {}

    protected:
        Foil const *m_pRefFoil;
        Foil *m_pBufferFoil;

        FoilWt *m_pFoilWt;

        bool m_bModified;

        QPalette m_Palette, m_SliderPalette;

        QPushButton *m_ppbMenuBtn;

        static Grid s_Grid; /** only for static saving and loading settings */

        static QByteArray s_Geometry;
};

