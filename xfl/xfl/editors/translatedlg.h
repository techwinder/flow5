/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xflgeom/geom3d/vector3d.h>
#include <xflwidgets/customdlg/xfldialog.h>

class FloatEdit;

class TranslateDlg : public XflDialog
{
    public:
        TranslateDlg(QWidget *pParent=nullptr);

        Vector3d const &translationVector() const {return m_Translation;}

    private:
        void setupLayout();
        void accept() override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private:

        Vector3d m_Translation;
        FloatEdit *m_pdeX, *m_pdeY, *m_pdeZ;

        static QByteArray s_WindowGeometry;
};

