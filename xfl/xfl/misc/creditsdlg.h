/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QDialog>

class CreditsDlg : public QDialog
{
    Q_OBJECT

    public:
        CreditsDlg(QWidget *pParent);
    private:
        void setupLayout();
};


