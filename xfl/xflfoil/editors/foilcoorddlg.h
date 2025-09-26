/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSplitter>

#include <xflfoil/editors/foildlg.h>


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

