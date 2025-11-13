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

#define _MATH_DEFINES_DEFINED


#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QGroupBox>

#include <TopTools_ListIteratorOfListOfShape.hxx>

#include "sailcadreaderdlg.h"
#include <xflcore/flow5events.h>
#include <xflcore/saveoptions.h>
#include <xflocc/occ_globals.h>
#include <xflwidgets/customwts/plaintextoutput.h>

bool SailCadReaderDlg::s_bShell = true;
QByteArray SailCadReaderDlg::s_WindowGeometry;

SailCadReaderDlg::SailCadReaderDlg(QWidget *pParent) : QDialog(pParent)
{
    setupLayout();
}


void SailCadReaderDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QPushButton *ppbImport = new QPushButton("Select file");
        connect(ppbImport, SIGNAL(clicked()), SLOT(onImportFile()));
        QGroupBox *pObjectsBox = new QGroupBox("Objects");
        {
            QHBoxLayout *pObjectsLayout = new QHBoxLayout;
            {
                QLabel *pLabBuild = new QLabel("Build one sail for each:");
                m_prbFACE  = new QRadioButton("Face");
                m_prbSHELL = new QRadioButton("Shell");
                pObjectsLayout->addWidget(pLabBuild);
                pObjectsLayout->addWidget(m_prbSHELL);
                pObjectsLayout->addWidget(m_prbFACE);
                pObjectsLayout->addStretch();
            }
            pObjectsBox->setLayout(pObjectsLayout);
        }

        m_pptoOutput = new PlainTextOutput;
        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
        }
        pMainLayout->addWidget(ppbImport);
        pMainLayout->addWidget(pObjectsBox);
        pMainLayout->addWidget(m_pptoOutput);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void SailCadReaderDlg::initDialog()
{
    m_prbFACE->setChecked(!s_bShell);
    m_prbSHELL->setChecked(s_bShell);
}


void SailCadReaderDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)     accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}


void SailCadReaderDlg::onImportFile()
{
    double dimension=0;
    QString logmsg;

    QString filter = "CAD Files (*.brep *stp *step *igs *iges)";
    QString filename = QFileDialog::getOpenFileName(nullptr, "CAD file", SaveOptions::CADDirName(),filter);
    if(!filename.length()) return;

    m_pptoOutput->onAppendThisPlainText("Reading file "+filename+"\n");

    QApplication::setOverrideCursor(Qt::WaitCursor);
    occ::importCADShapes(filename, m_ListOfShape, dimension, 1.e-4, logmsg, this);
    QApplication::restoreOverrideCursor();
    m_pptoOutput->onAppendThisPlainText(logmsg);


    logmsg.clear();
    int ishape=0;
    for(TopTools_ListIteratorOfListOfShape shellit(m_ListOfShape); shellit.More(); shellit.Next())
    {
        m_pptoOutput->onAppendThisPlainText(QString::asprintf("Shape %d:\n", ishape));
        occ::listShapeContent(shellit.Value(), logmsg, "   ");
        m_pptoOutput->onAppendThisPlainText(logmsg+"\n");
        ishape++;
    }
}


void SailCadReaderDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent const *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        m_pptoOutput->onAppendThisPlainText(pMsgEvent->msg());

    }
    else
        QDialog::customEvent(pEvent);
}


void SailCadReaderDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);

}


void SailCadReaderDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}


void SailCadReaderDlg::accept()
{
    s_bShell = m_prbSHELL->isChecked();
    QDialog::accept();
}


