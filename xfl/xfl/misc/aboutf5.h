/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>

class AboutFlow5 : public QDialog
{
    Q_OBJECT

    public:
        AboutFlow5(QWidget *pParent);


    private:
        void setupLayout();
};


