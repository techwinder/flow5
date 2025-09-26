/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QRadioButton>


#include <xflwidgets/customdlg/xfldialog.h>
#include <xflgeom/geom3d/vector3d.h>

class FloatEdit;

class RotateDlg : public XflDialog
{
    public:
        RotateDlg(QWidget *pParent);

        int axis() const;
        double angle() const;

    private:
        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private:

        QRadioButton *m_prbX, *m_prbY, *m_prbZ;
        FloatEdit *m_pfeAngle;

        static QByteArray s_WindowGeometry;
        static double s_Angle;
        static int s_iAxis;
};


