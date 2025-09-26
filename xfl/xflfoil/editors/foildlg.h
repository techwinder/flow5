/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QFrame>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QPoint>
#include <QSize>
#include <QSettings>

#include <xflcore/fontstruct.h>
#include <xflwidgets/view/grid.h>
#include <xflwidgets/customdlg/xfldialog.h>

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

