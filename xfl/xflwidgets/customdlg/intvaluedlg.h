/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>
#include <xflwidgets/customdlg/xfldialog.h>


class IntEdit;

class IntValueDlg : public XflDialog
{
    public:
        IntValueDlg(QWidget *pParent=nullptr);

        void setValue(int intvalue);
        int value() const;
        void setLeftLabel(QString const &label) {m_pLeftLabel->setText(label);}
        void setRightLabel(QString const &label) {m_pRightLabel->setText(label);}

    private:
        void setupLayout();

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *) override;


    private:
        QLabel *m_pLeftLabel, *m_pRightLabel;
        IntEdit *m_pieIntEdit;
        static QByteArray s_WindowGeometry;
};



