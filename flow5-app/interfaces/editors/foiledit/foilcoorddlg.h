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
#include <QPushButton>
#include <QSplitter>

#include <interfaces/editors/foiledit/foildlg.h>


class FoilWt;
class Foil;
class PlainTextOutput;

class FoilCoordDlg : public FoilDlg
{
    Q_OBJECT

    public:
        FoilCoordDlg(QWidget *pParent=nullptr);

        void initDialog(Foil *pFoil) override;

    private slots:
        void onReset() override;
        void onApply() override;

    private:
        void fillCoordinates();
        void readCoordinates();

        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private:
        PlainTextOutput *m_pCoords;
        QSplitter *m_pHSplitter;

        static QByteArray s_HSplitterSizes;
};

