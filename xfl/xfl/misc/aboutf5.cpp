/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QDialogButtonBox>
#include <QBitmap>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>


#include "aboutf5.h"

#include <xflcore/xflcore.h>

AboutFlow5::AboutFlow5(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("About flow5");
    setupLayout();
}


void AboutFlow5::setupLayout()
{
    QVBoxLayout *pLogoLayout = new QVBoxLayout;
    {
        QLabel *plabIconF5 = new QLabel;
        plabIconF5->setObjectName("flow5");
        plabIconF5->setPixmap(QPixmap(QString::fromUtf8(":/images/flow5_1287x429.png")).scaled(QSize(500,300), Qt::KeepAspectRatio));
        plabIconF5->setAlignment(Qt::AlignCenter);
        QLabel *plab1  = new QLabel(xfl::versionName(true));
        plab1->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);
        QLabel *pFlow5Link = new QLabel;
        pFlow5Link->setText("<a href=https://flow5.tech>https://flow5.tech</a>");
        pFlow5Link->setOpenExternalLinks(true);
        pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
        pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

        pLogoLayout->addWidget(plabIconF5);
        pLogoLayout->addWidget(plab1);
        pLogoLayout->addWidget(pFlow5Link);
    }

    QString copyright = QString::fromUtf8("Copyright © André Deperrois - All rights reserved");
    QLabel *plab4  = new QLabel(copyright);
    QString trademark = QString::fromUtf8("flow5® is a product and a registered trademark of the company Vic-Aéro");
    QLabel *plab3  = new QLabel(trademark);
    QLabel *plab6  = new QLabel(  "This program is distributed in the hope that it will be useful\n"
                                  "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                                  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.");


    QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    {
        connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pLogoLayout);

        pMainLayout->addWidget(plab3);
        pMainLayout->addWidget(plab4);
        pMainLayout->addSpacing(17);
        pMainLayout->addStretch();
        pMainLayout->addWidget(plab6);
        pMainLayout->addSpacing(17);
        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);
}



