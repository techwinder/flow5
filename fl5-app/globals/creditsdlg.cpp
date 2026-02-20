/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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
#ifdef INTEL_MKL
#include <mkl_service.h>
#endif

#include <Standard_Version.hxx>

#ifdef NO_GMSH
#define GMSH_API_VERSION "Disabled"
#else
#include <gmsh.h>
#endif

CreditsDlg::CreditsDlg(QWidget *pParent) : QDialog(pParent)
{
    setupLayout();
}


void CreditsDlg::setupLayout()
{
    QLabel *plab6  = new QLabel(tr("This application uses the following licensed libraries and technology\n"));
    plab6->setStyleSheet("font-weight: bold");

    QGroupBox *pXFoilBox = new QGroupBox("XFoil");
    {
        QVBoxLayout *pXFoilLayout = new QVBoxLayout;
        {
            QLabel *plabVersion = new QLabel(tr("<p><b>Version: </b>6.94 ported to C language</p>") );
            QLabel *plabXFoil  = new QLabel(tr("<p>XFOIL is an interactive program for the design and analysis of subsonic isolated airfoils.</p>"));
            plabXFoil->setWordWrap(true);
            QLabel *plabAuthor = new QLabel(tr("<p><b>Author: </b>Mark Drela (also Harold Youngren)</p>"));

            QLabel *plabXFoilLink = new QLabel(tr("<a href=https://web.mit.edu/drela/Public/web/xfoil>XFoil web page</a>"));
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

            QLabel *plabVersion = new QLabel(tr("<p><b>Version:</b> ") + QString(OCC_VERSION_COMPLETE));
            QLabel *plabDescription = new QLabel(tr("<p>Open Cascade Technology (OCCT) is an open-source software "
                                                 "development platform for 3D CAD, CAM, CAE, etc. that is developed "
                                                 "and supported by Open Cascade SAS.</p>"));
            plabDescription->setWordWrap(true);
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
    QGroupBox *pGmshBox = new QGroupBox("Gmsh");
    {
        QVBoxLayout *pGmshLayout = new QVBoxLayout;
        {
            QLabel *plabVersion = new QLabel(tr("<p><b>Version: </b>") + QString(GMSH_API_VERSION) + "</p>");
            QLabel *plabGmshLink = new QLabel;
            plabGmshLink->setText("<a href=https://gmsh.info/doc/preprints/gmsh_paper_preprint.pdf>C. Geuzaine and J.-F. Remacle.<br>"
                                  "Gmsh: a three-dimensional finite element mesh generator with built-in pre- and post-processing facilities.<br>"
                                  "International Journal for Numerical Methods in Engineering 79(11), pp. 1309-1331, 2009</a>");
            plabGmshLink->setOpenExternalLinks(true);
            plabGmshLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
            plabGmshLink->setWordWrap(true);

            pGmshLayout->addWidget(plabVersion);
            pGmshLayout->addWidget(plabGmshLink);
        }
        pGmshBox->setLayout(pGmshLayout);
    }
#ifdef INTEL_MKL
    QGroupBox *pMKLBox = new QGroupBox("Intel MKL Library");
    {
        QVBoxLayout *pMKLLayout = new QVBoxLayout;
        {
            MKLVersion Version;

            mkl_get_version(&Version);

            QString strange;
            strange += QString::asprintf("<p><b>%s</b>%d.%d.%d<br>", tr("Version: ").toLatin1().data(), Version.MajorVersion, Version.MinorVersion, Version.UpdateVersion);
            strange += QString::asprintf("<b>%s</b> %s", tr("Processor optimization: ").toLatin1().data(), Version.Processor) + "</p>";

            QLabel *plabVersion = new QLabel(strange);

            QLabel *pMKLLink = new QLabel;
            pMKLLink->setText("<a href=https://software.intel.com/en-us/mkl>https://software.intel.com/en-us/mkl</a>");
            pMKLLink->setOpenExternalLinks(true);
            pMKLLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
            pMKLLink->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

            pMKLLayout->addWidget(plabVersion);
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
        pMainLayout->addWidget(plab6);
        pMainLayout->addWidget(pXFoilBox);
        pMainLayout->addWidget(pOccBox);
        pMainLayout->addWidget(pGmshBox);
#ifdef INTEL_MKL
        pMainLayout->addWidget(pMKLBox);
#endif
        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);
//    setMinimumHeight(400);
}
