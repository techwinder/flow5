/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QDialogButtonBox>

#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>

class IntEdit;
class FloatEdit;

class BodyScaleDlg : public QDialog
{
    Q_OBJECT

    public:
        BodyScaleDlg(QWidget *pParent=nullptr);
        void initDialog(bool bFrameOnly=false);

        double XFactor() const {return m_XFactor;}
        double YFactor() const {return m_YFactor;}
        double ZFactor() const {return m_ZFactor;}

        bool bFrameOnly() const {return m_bFrameOnly;}
        void setFrameOnly(bool bFrameOnly) {m_bFrameOnly=bFrameOnly;}
        void setFrameIndex(int iFrame) {m_FrameID = iFrame;}
        void enableFrameID(bool bEnable);

    private slots:
        void onOK();
        void onRadioBtn();
        void onEditingFinished();
        void onButton(QAbstractButton*pButton);

    private:
        void setupLayout();
        void enableControls();
        void keyPressEvent(QKeyEvent *event) override;

    private:

        QDialogButtonBox *m_pButtonBox;


        QRadioButton *m_prbBody, *m_prbFrame;
        FloatEdit *m_pdeXScaleFactor;
        FloatEdit *m_pdeYScaleFactor;
        FloatEdit *m_pdeZScaleFactor;
        IntEdit *m_pieFrameID;


    private:
        double m_XFactor, m_YFactor, m_ZFactor;
        bool m_bFrameOnly;
        int m_FrameID;
};

