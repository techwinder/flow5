/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include "helpimgdlg.h"

#include <QScrollArea>
#include <QVBoxLayout>

QByteArray HelpImgDlg::s_Geometry;

HelpImgDlg::HelpImgDlg(const QString &imagepath, QWidget *pParent) : QDialog(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setModal(false);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pHelpLab = new QLabel();
        m_pHelpLab->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
        m_Image.load(imagepath);
        m_pHelpLab->setPixmap(m_Image);

        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        {
            connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }
        pMainLayout->addWidget(m_pHelpLab);
        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMinimumSize(QSize(650,900)); // sizeHint and minimumSizeHint fail
}

/*
QSize HelpImgDlg::sizeHint() const
{
    if(!m_Image.isNull())
    {
        QSize sz = m_Image.size();
        if(sz.width()>1000)
        {
            sz.scale(1000,800, Qt::KeepAspectRatio);
        }
        else if(sz.height()>800)
        {
            sz.scale(sz.width()*800/sz.height(), 800, Qt::KeepAspectRatio);
        }
        return sz;
    }
    return QSize(750,700);
}*/


void HelpImgDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void HelpImgDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void HelpImgDlg::resizeEvent(QResizeEvent *pEvent)
{
    QDialog::resizeEvent(pEvent);
    m_pHelpLab->setPixmap(m_Image.scaled(m_pHelpLab->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

