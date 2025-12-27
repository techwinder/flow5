/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois 
    
    This file is part of flow5.

    flow5 is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    flow5 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flow5.
    If not, see <https://www.gnu.org/licenses/>.


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

