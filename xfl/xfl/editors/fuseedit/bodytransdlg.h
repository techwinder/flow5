/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QLabel>


#include <xflgeom/geom3d/vector3d.h>

class IntEdit;
class FloatEdit;
class BodyTransDlg : public QDialog
{
    Q_OBJECT

    public:
        BodyTransDlg(QWidget *pParent=nullptr);
        void initDialog();

        double dx() const {return m_XTrans;}
        double dy() const {return m_YTrans;}
        double dz() const {return m_ZTrans;}
        Vector3d translation() const {return Vector3d(m_XTrans, m_YTrans, m_ZTrans);}

        void enableDirections(bool bx, bool by, bool bz);

        bool bFrameOnly() const {return m_bFrameOnly;}
        void setFrameOnly(bool bFrameOnly) {m_bFrameOnly=bFrameOnly;}
        void setFrameId(int id);
        void enableFrameID(bool bEnable);
        void checkFrameId(bool bForce);


    private slots:
        void onOK();
        void onFrameOnly();
        void onButton(QAbstractButton*pButton);

    private:
        void keyPressEvent(QKeyEvent *event);
        void setupLayout();

        QDialogButtonBox *m_pButtonBox;

        FloatEdit *m_pdeXTransFactor;
        FloatEdit *m_pdeYTransFactor;
        FloatEdit *m_pdeZTransFactor;
        IntEdit *m_pieFrameID;
        QCheckBox *m_pchFrameOnly;


        double m_XTrans, m_YTrans, m_ZTrans;
        bool   m_bFrameOnly;
        int    m_FrameID;

};

