/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once
#include <QCheckBox>

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>

class IntEdit;
class FloatEdit;


class ImageDlg : public QDialog
{
    Q_OBJECT
    public:
        ImageDlg(QWidget *pParent, QVector<double> values, bool bScale, bool bFlipH, bool bFlipV);

        bool bChanged() const {return m_bChanged;}

        QPointF offset();
        double xScale();
        double yScale();

        bool bScaleWithView() const {return m_pchScaleWithView->isChecked();}
        bool bFlipH()         const {return m_pchFlipH->isChecked();}
        bool bFlipV()         const {return m_pchFlipV->isChecked();}

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void connectSignals();

    signals:
        void imageChanged(bool,bool,bool,QPointF,double,double);

    private slots:
        void onButton(QAbstractButton *pButton);
        void onApply();

    private:
        QWidget *m_pParent;

        bool m_bChanged;

        IntEdit *m_pieXOffset, *m_pieYOffset;
        FloatEdit *m_pfeXScale, *m_pfeYScale;
        QCheckBox *m_pchScaleWithView;
        QCheckBox *m_pchFlipH, *m_pchFlipV;
        QDialogButtonBox *m_pButtonBox;
};


