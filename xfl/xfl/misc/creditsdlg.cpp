/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>

#include "creditsdlg.h"
#include <xflcore/xflcore.h>
#ifdef INTEL_MKL
#include <mkl_service.h>
#endif
#include <Standard_Version.hxx>

CreditsDlg::CreditsDlg(QWidget *pParent) : QDialog(pParent)
{
    setupLayout();
}


void CreditsDlg::setupLayout()
{
    QLabel *pLab6  = new QLabel("This program uses the following licensed libraries and technology\n");
    pLab6->setStyleSheet("font-weight: bold");

    QGroupBox *pOccBox = new QGroupBox("Open Cascade Technology");
    {
        QVBoxLayout *pOccLayout = new QVBoxLayout;
        {
            QLabel *pLabIconOcc = new QLabel;
            pLabIconOcc->setObjectName("Occ");
            pLabIconOcc->setPixmap(QPixmap(QString::fromUtf8(":/icons/Occ_logo.png")));
            pLabIconOcc->setAlignment(Qt::AlignCenter);

            QLabel *plabVersion = new QLabel("Version: " + QString(OCC_VERSION_COMPLETE));
            QLabel *plabDescription = new QLabel("<p>Open Cascade Technology (OCCT) is an open-source software<br>"
                                                 "development platform for 3D CAD, CAM, CAE, etc. that is developed<br>"
                                                 "and supported by Open Cascade SAS.</p>");
            QLabel *pOccLink = new QLabel;
            pOccLink->setText("<a href=https://www.opencascade.com>https://www.opencascade.com</a>");
            pOccLink->setOpenExternalLinks(true);
            pOccLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
            pOccLink->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

            pOccLayout->addWidget(pLabIconOcc);
            pOccLayout->addWidget(plabVersion);
            pOccLayout->addWidget(plabDescription);
            pOccLayout->addWidget(pOccLink);
        }
        pOccBox->setLayout(pOccLayout);
    }
#ifdef INTEL_MKL
    QGroupBox *pMKLBox = new QGroupBox("Intel MKL Library");
    {
        QVBoxLayout *pMKLLayout = new QVBoxLayout;
        {
            MKLVersion Version;

            mkl_get_version(&Version);
            QString strange;

            strange += QString::asprintf("<p>Version:           %d.%d.%d<br>", Version.MajorVersion, Version.MinorVersion, Version.UpdateVersion);
            strange += QString::asprintf("Processor optimization:  %s</p>", Version.Processor);

            QLabel *plabIconMKL = new QLabel;
            plabIconMKL->setObjectName("MKL");
            plabIconMKL->setPixmap(QPixmap(QString::fromUtf8(":/icons/icon_mkl.png")));
            plabIconMKL->setAlignment(Qt::AlignCenter);
            QLabel *plabDescription = new QLabel("<p>Intel's Math Kernel Library (Intel MKL) is a library of optimized math<br>"
                                                 "routines for science, engineering, and financial applications. Core math<br>"
                                                 "functions include BLAS, LAPACK, ScaLAPACK, sparse solvers, fast Fourier<br>"
                                                 "transforms, and vector math. The routines in MKL are hand-optimized<br>"
                                                 "specifically for Intel processors.<br><br>"
                                                 "The library supports Intel processors and is available for Windows,<br>"
                                                 "Linux and macOS operating systems.</p>");

            QLabel *plabVersion = new QLabel(strange);

            QLabel *pMKLLink = new QLabel;
            pMKLLink->setText("<a href=https://software.intel.com/en-us/mkl>https://software.intel.com/en-us/mkl</a>");
//            pMKLLink->setText("<a href=http://www.flow5.tech>http://www.flow5.tech</a>");

            pMKLLink->setOpenExternalLinks(true);
            pMKLLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
            pMKLLink->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

            pMKLLayout->addWidget(plabIconMKL);
            pMKLLayout->addWidget(plabVersion);
            pMKLLayout->addWidget(plabDescription);
            pMKLLayout->addWidget(pMKLLink);
        }
        pMKLBox->setLayout(pMKLLayout);
    }
#endif


    QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    {
        connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addStretch(1);
        pMainLayout->addSpacing(17);
        pMainLayout->addWidget(pLab6);
        pMainLayout->addWidget(pOccBox);
#ifdef INTEL_MKL
        pMainLayout->addWidget(pMKLBox);
#endif
        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);
//    setMinimumHeight(400);
}
