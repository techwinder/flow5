/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>

#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customdlg/xfldialog.h>

class DoubleValueDlg : public XflDialog
{
    public:
        DoubleValueDlg(QWidget *pParent, QVector<double> values, QStringList const &leftlabels, QStringList const &rightlabels);

        void setValue(int iVal, int value) {if(iVal>=0 && iVal<m_pDoubleEdit.size()) m_pDoubleEdit[iVal]->setValue(value);}
        double value(int iVal) const{if(iVal>=0 && iVal<m_pDoubleEdit.size()) return m_pDoubleEdit[iVal]->value(); else return 0;}

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        static QByteArray s_WindowGeometry;

    private:
        QVector<FloatEdit *> m_pDoubleEdit;
};






