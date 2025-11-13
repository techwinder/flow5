/****************************************************************************

    flow5
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

    QGroupBox *pXFoilBox = new QGroupBox("XFoil");
    {
        QVBoxLayout *pXFoilLayout = new QVBoxLayout;
        {
            QLabel *plabVersion = new QLabel("<p><b>Version: </b>6.94 ported to C language</p>" );
            QLabel *plabXFoil  = new QLabel("<p>XFOIL is an interactive program for the design and analysis of subsonic isolated airfoils.</p>");
            plabXFoil->setWordWrap(true);
            QLabel *plabAuthor = new QLabel("<p><b>Author: </b>Mark Drela (also Harold Youngren)</p>");

            QLabel *plabXFoilLink = new QLabel("<a href=https://web.mit.edu/drela/Public/web/xfoil>XFoil web page</a>");
            plabXFoilLink->setOpenExternalLinks(true);
            plabXFoilLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

            pXFoilLayout->addWidget(plabXFoil);
            pXFoilLayout->addWidget(plabVersion);
            pXFoilLayout->addWidget(plabAuthor);
            pXFoilLayout->addWidget(plabXFoilLink);
        }
        pXFoilBox->setLayout(pXFoilLayout);
    }

    QGroupBox *pOccBox = new QGroupBox("Open Cascade Technology");
    {
        QVBoxLayout *pOccLayout = new QVBoxLayout;
        {
            QLabel *plabVersion = new QLabel("<p><b>Version: </b>" + QString(OCC_VERSION_COMPLETE) + "</p>");
            QLabel *plabDescription = new QLabel("<p>Open Cascade Technology (OCCT) is an open-source software<br>"
                                                 "development platform for 3D CAD, CAM, CAE, etc. that is developed<br>"
                                                 "and supported by Open Cascade SAS.</p>");
            QLabel *pOccLink = new QLabel;
            pOccLink->setText("<a href=https://www.opencascade.com>https://www.opencascade.com</a>");
            pOccLink->setOpenExternalLinks(true);
            pOccLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
            pOccLink->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

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

            strange += QString::asprintf("<p><b>Version: </b>          %d.%d.%d<br>", Version.MajorVersion, Version.MinorVersion, Version.UpdateVersion);
            strange += QString::asprintf("Processor optimization:  %s</p>", Version.Processor);

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
        pMainLayout->addWidget(pXFoilBox);
        pMainLayout->addWidget(pOccBox);
#ifdef INTEL_MKL
        pMainLayout->addWidget(pMKLBox);
#endif
        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);
//    setMinimumHeight(400);
}
