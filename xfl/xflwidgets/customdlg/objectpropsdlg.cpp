/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include <QShowEvent>
#include <QHideEvent>

#include "objectpropsdlg.h"


#include <xflwidgets/customwts/plaintextoutput.h>

QByteArray ObjectPropsDlg::s_Geometry;


ObjectPropsDlg::ObjectPropsDlg(QWidget *pParent) : QDialog(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupLayout();
}


void ObjectPropsDlg::setupLayout()
{
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    {
        connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }
    QVBoxLayout * pMainLayout = new QVBoxLayout(this);
    {
        m_ppto = new PlainTextOutput;
        pMainLayout->addWidget(m_ppto);
        pMainLayout->addSpacing(20);
        pMainLayout->addWidget(pButtonBox);
    }

    setLayout(pMainLayout);
}


void ObjectPropsDlg::initDialog(QString const &title, QString const &props)
{
    setWindowTitle(title);
    m_ppto->insertPlainText(props);
    m_ppto->moveCursor(QTextCursor::Start);
}


void ObjectPropsDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void ObjectPropsDlg::hideEvent(QHideEvent*pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}







