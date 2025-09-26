/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

#include "doublevaluedlg.h"


QByteArray DoubleValueDlg::s_WindowGeometry;

DoubleValueDlg::DoubleValueDlg(QWidget *pParent, QVector<double> values, QStringList const &leftlabels, QStringList const &rightlabels) : XflDialog(pParent)
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGridLayout *pIntLayout = new QGridLayout;
        {
            m_pDoubleEdit.clear();
            for(int i=0; i<values.size(); i++)
            {
                QLabel *pLeftLabel = new QLabel;
                if(i<leftlabels.size()) pLeftLabel->setText(leftlabels.at(i));
                m_pDoubleEdit.push_back(new FloatEdit);
                QLabel *pRightLabel = new QLabel;
                if(i<rightlabels.size()) pRightLabel->setText(rightlabels.at(i));

                pIntLayout->addWidget(pLeftLabel,           i, 1);
                pIntLayout->addWidget(m_pDoubleEdit.last(), i, 2);
                pIntLayout->addWidget(pRightLabel,          i, 3);
            }
        }

        m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        pMainLayout->addLayout(pIntLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
    for(int i=0; i<values.size(); i++)
        m_pDoubleEdit[i]->setValue(values.at(i));
}


void DoubleValueDlg::showEvent(QShowEvent *pEvent)
{
    XflDialog::showEvent(pEvent);

    restoreGeometry(s_WindowGeometry);

    if(m_pDoubleEdit.size())
    {
        m_pDoubleEdit.first()->selectAll();
        m_pDoubleEdit.first()->setFocus();
    }
}


void DoubleValueDlg::hideEvent(QHideEvent *pEvent)
{
    XflDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}



void DoubleValueDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
            }
            else
            {
                QDialog::accept();
            }
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            pEvent->ignore();
    }
}

