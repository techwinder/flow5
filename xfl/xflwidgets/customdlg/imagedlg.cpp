/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

#include "imagedlg.h"
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/view/section2dwt.h>
//#include <xfl3d/views/gl3dview.h>


ImageDlg::ImageDlg(QWidget *pParent, QVector<double> values, bool bScale, bool bFlipH, bool bFlipV) : QDialog(pParent)
{
    m_pParent = pParent;

    m_bChanged = false;
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGridLayout *pIntLayout = new QGridLayout;
        {
            m_pieXOffset = new IntEdit( int(values.at(0)));
            m_pieYOffset = new IntEdit(-int(values.at(1)));
            m_pfeXScale = new FloatEdit(values.at(2));
            m_pfeYScale = new FloatEdit(values.at(3));
            m_pchScaleWithView = new QCheckBox("Scale with view");
            m_pchScaleWithView->setChecked(bScale);

            m_pchFlipH = new QCheckBox("Flip horizontally");
            m_pchFlipH->setChecked(bFlipH);
            m_pchFlipV = new QCheckBox("Flip vertically");
            m_pchFlipV->setChecked(bFlipV);

            pIntLayout->addWidget(new QLabel("x-offset"), 1, 1);
            pIntLayout->addWidget(m_pieXOffset,           1, 2);
            pIntLayout->addWidget(new QLabel("pixels"),   1, 3);
            pIntLayout->addWidget(new QLabel("y-offset"), 2, 1);
            pIntLayout->addWidget(m_pieYOffset,           2, 2);
            pIntLayout->addWidget(new QLabel("pixels"),   2, 3);
            pIntLayout->addWidget(new QLabel("x-scale"),  3, 1);
            pIntLayout->addWidget(m_pfeXScale,            3, 2);
            pIntLayout->addWidget(new QLabel("y-scale"),  4, 1);
            pIntLayout->addWidget(m_pfeYScale,            4, 2);
            pIntLayout->addWidget(m_pchFlipH,             5, 1, 1, 2);
            pIntLayout->addWidget(m_pchFlipV,             6, 1, 1, 2);
            pIntLayout->addWidget(m_pchScaleWithView,     7, 1, 1, 2);
        }

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Apply, this);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        }

        pMainLayout->addLayout(pIntLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);

    connectSignals();
}


void ImageDlg::connectSignals()
{
    connect(m_pieXOffset,        SIGNAL(intChanged(int)),     SLOT(onApply()));
    connect(m_pieYOffset,        SIGNAL(intChanged(int)),     SLOT(onApply()));
    connect(m_pfeXScale,         SIGNAL(floatChanged(float)), SLOT(onApply()));
    connect(m_pfeYScale,         SIGNAL(floatChanged(float)), SLOT(onApply()));
    connect(m_pchFlipH,          SIGNAL(clicked(bool)),       SLOT(onApply()));
    connect(m_pchFlipV,          SIGNAL(clicked(bool)),       SLOT(onApply()));
    connect(m_pchScaleWithView,  SIGNAL(clicked(bool)),       SLOT(onApply()));
}


void ImageDlg::keyPressEvent(QKeyEvent *pEvent)
{
    // Prevent Return Key from closing App
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            m_pButtonBox->button(QDialogButtonBox::Close)->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        default:
            pEvent->ignore();
    }
    QDialog::keyPressEvent(pEvent);
}


void ImageDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)  accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Apply) == pButton)  onApply();
}


void ImageDlg::onApply()
{
    emit imageChanged(bScaleWithView(), bFlipH(), bFlipV(), offset(), xScale(), yScale());
}


QPointF ImageDlg::offset()
{
    return QPointF(m_pieXOffset->value(), -m_pieYOffset->value());
}


double ImageDlg::xScale()
{
    return m_pfeXScale->value();
}


double ImageDlg::yScale()
{
    return m_pfeYScale->value();
}



