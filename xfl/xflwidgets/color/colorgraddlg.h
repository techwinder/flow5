/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QFrame>
#include <QRadioButton>
#include <QDialogButtonBox>

class ColorBtn;

class ColorGradDlg : public QDialog
{
    Q_OBJECT
    public:
        ColorGradDlg(QVector<QColor>const &clrs, QWidget *parent = nullptr);

    public slots:
        void onColorBtn();
        void onNColors();
        void onButton(QAbstractButton *pButton);
        QVector<QColor> &colours() {return  m_Clr;}

    protected:
        QSize sizeHint() const override {return QSize(350,550);}

    private:
        QColor colour(float tau) const;
        void makeCtrlFrameLayout();
        void setupLayout();
        void updateColouredFrame();

    private:

        QFrame *m_pDiscreteClrFrame;
        QWidget *m_pTestClrFrame;

        QVector<ColorBtn*> m_pColorBtn;
        QVector<QColor> m_Clr;

        QRadioButton *m_prb2Colors;
        QRadioButton *m_prb3Colors;
        QDialogButtonBox *m_pButtonBox;
};

