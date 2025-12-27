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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDir>


#include "streamtestdlg.h"
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>

#include <api/fl5color.h>

QByteArray StreamTestDlg::s_Geometry;


StreamTestDlg::StreamTestDlg(QWidget *pParent) : QDialog(pParent)
{
    setupLayout();
    setMinimumSize(700,700);
}


void StreamTestDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_ppto = new PlainTextOutput;



        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        {
            m_ppbSave = new QPushButton("Save");
            m_ppbLoad = new QPushButton("Load");

            m_pButtonBox->addButton(m_ppbSave, QDialogButtonBox::ActionRole);
            m_pButtonBox->addButton(m_ppbLoad, QDialogButtonBox::ActionRole);

            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        }

        pMainLayout->addWidget(m_ppto);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void StreamTestDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)  reject();
    else if (m_ppbSave == pButton)  save();
    else if (m_ppbLoad == pButton)  load();
}


void StreamTestDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void StreamTestDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


std::string StreamTestDlg::tempFilePath()
{
    std::string temppath;
    temppath = std::filesystem::temp_directory_path().string();
    temppath +=            std::filesystem::path::preferred_separator;
    temppath += "stream.bin";
    std::cout <<  temppath << std::endl;
    return temppath;
}


std::ostream& operator - (std::ostream& outstream, const int &n)
{
    outstream.write(reinterpret_cast<const char*>(&n), sizeof(int));
    std::cout << "writing..."<<std::endl;
    return outstream;
}

std::istream& operator + (std::istream& instream, int &n)
{
    instream.read(reinterpret_cast<char*>(&n), sizeof(int));
    std::cout << "reading..."<<std::endl;
    return instream;
}

void StreamTestDlg::save()
{
    std::string filename = tempFilePath();
    m_ppto->onAppendStdText("Saving to "+filename+'\n');

    std::ofstream outstream(filename, std::ofstream::binary);
    if (!outstream.is_open())
    {
        m_ppto->onAppendQText("Error: Failed to open file for writing.\n");
        return;
    }

    int r(233), g(131), b(81), a(151);
    m_ppto->onAppendQText(QString::asprintf("saving r=%d, g=%d, b=%d, a=%d\n", r,g,b,a));
    outstream - r - g - b - a;
/*    outstream.write(reinterpret_cast<const char *>(&r), sizeof(int));
    outstream.write(reinterpret_cast<const char *>(&g), sizeof(int));
    outstream.write(reinterpret_cast<const char *>(&b), sizeof(int));
    outstream.write(reinterpret_cast<const char *>(&a), sizeof(int));*/


    outstream.close();
    m_ppto->onAppendQText("Serialization successful.\n\n");
}


void StreamTestDlg::load()
{
    std::string filename = tempFilePath();
    m_ppto->onAppendStdText("Loading from "+filename+'\n');

    std::ifstream instream(filename, std::ios::binary);
    if (!instream.is_open())
    {
        m_ppto->onAppendQText("Error: Failed to open file for reading.\n");
        return;
    }

    int r(17),g(457),b(255),a(39);
    instream + r + g + b + a;
/*    instream.read(reinterpret_cast<char *>(&r), sizeof(int));
    instream.read(reinterpret_cast<char *>(&g), sizeof(int));
    instream.read(reinterpret_cast<char *>(&b), sizeof(int));
    instream.read(reinterpret_cast<char *>(&a), sizeof(int));*/


    instream.close();

    m_ppto->onAppendQText(QString::asprintf("loading r=%d, g=%d, b=%d, a=%d\n", r,g,b,a));

    m_ppto->onAppendQText("De-serialization successful.\n\n");

}

