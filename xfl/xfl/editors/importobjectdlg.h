/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QListWidget>
#include <xflwidgets/customdlg/xfldialog.h>

class ImportObjectDlg : public XflDialog
{
    Q_OBJECT

    public:
        ImportObjectDlg(QWidget *pParent, bool bWings);

        QString planeName()  const;
        QString objectName() const;

        QSize sizeHint() const override {return QSize(1000,1000);}

    private slots:
        void onPlaneChanged();

    private:
        void setupLayout(bool bWings);
        void fillObjects();

    private:
        QListWidget *m_plvPlanes;
        QListWidget *m_plvObjects;

        bool m_bWings;
};
