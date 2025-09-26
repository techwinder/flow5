/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xfl/editors/boatedit/externalsaildlg.h>


class STLSailDlg : public ExternalSailDlg
{
    Q_OBJECT
    public:
        STLSailDlg(QWidget *pParent);
        void initDialog(Sail *pSail) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

    private:
        void setupLayout();

    private slots:
        void onTabChanged();

    private:
};


