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




#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>

#include "textoutputtestdlg.h"
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <core/displayoptions.h>

QByteArray TextOutputTestDlg::s_Geometry;


TextOutputTestDlg::TextOutputTestDlg(QWidget *pParent) : QDialog(pParent)
{
    setupLayout();
    setMinimumSize(700,700);

    m_ppbPlain->setChecked(true);
    m_prbTextFnt->setChecked(true);
    m_pteOutput->setFont(DisplayOptions::textFont());
}


void TextOutputTestDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QVBoxLayout *pCharLayout = new QVBoxLayout;
        {
            QLabel *plabChar = new QLabel("Plain input:");
            m_pptInput = new PlainTextOutput;
            m_pptInput->setCharDimensions(1,11);
            m_pptInput->setReadOnly(false);
            QString strange("<p><i>This is</i> <b>html</b> <font color=green>colored</font> <u>text</u></p>"
                            "<p>&alpha; = 15&deg;</p>\n\n"
                            "# Markdown:\n\n"
                            "_This is_ **markdown** <span style=color:yellow>colored</span> *text*\n\n"
                            "&alpha; = 15&deg;\n\n"
                            "1. First item\n"
                            "7. Another item\n"
                            "    1. Indented item\n"
                            "    2. Indented item");
            m_pptInput->setPlainText(strange);

            pCharLayout->addWidget(plabChar);
            pCharLayout->addWidget(m_pptInput);
        }

        QHBoxLayout *pFormatLayout = new QHBoxLayout;
        {
            m_ppbPlain    = new QPushButton("to plain");
            m_ppbMarkDown = new QPushButton("to markdown");
            m_ppbHtml     = new QPushButton("to HTML");

            connect(m_ppbPlain,    SIGNAL(clicked()), SLOT(toPlainText()));
            connect(m_ppbMarkDown, SIGNAL(clicked()), SLOT(toMarkDown()));
            connect(m_ppbHtml,     SIGNAL(clicked()), SLOT(toHTML()));

            pFormatLayout->addWidget(m_ppbPlain);
            pFormatLayout->addWidget(m_ppbMarkDown);
            pFormatLayout->addWidget(m_ppbHtml);
            pFormatLayout->addStretch();
        }


        QVBoxLayout *pOutputLayout = new QVBoxLayout;
        {
            QHBoxLayout *pFontLayout = new QHBoxLayout;
            {
                QLabel *plabChar = new QLabel("Output:");
                m_prbTextFnt  = new QRadioButton("Text font ("  + DisplayOptions::textFont().family() +")");
                m_prbTableFnt = new QRadioButton("Table font (" + DisplayOptions::tableFont().family() +")");
                m_prbTreeFnt  = new QRadioButton("Tree font ("  + DisplayOptions::treeFont().family() +")");

                m_prbTextFnt->setFont( DisplayOptions::textFont());
                m_prbTableFnt->setFont(DisplayOptions::tableFont());
                m_prbTreeFnt->setFont( DisplayOptions::treeFont());

                connect(m_prbTextFnt,  SIGNAL(clicked()), SLOT(onOutputFont()));
                connect(m_prbTableFnt, SIGNAL(clicked()), SLOT(onOutputFont()));
                connect(m_prbTreeFnt,  SIGNAL(clicked()), SLOT(onOutputFont()));

                QButtonGroup *pGroup = new QButtonGroup;
                {
                    pGroup->addButton(m_prbTextFnt);
                    pGroup->addButton(m_prbTableFnt);
                    pGroup->addButton(m_prbTreeFnt);
                }
                pFontLayout->addWidget(plabChar);
                pFontLayout->addStretch();
                pFontLayout->addWidget(m_prbTextFnt);
                pFontLayout->addWidget(m_prbTableFnt);
                pFontLayout->addWidget(m_prbTreeFnt);
                pFontLayout->addStretch();
            }
            m_pteOutput = new QTextEdit;
            pOutputLayout->addLayout(pFontLayout);
            pOutputLayout->addWidget(m_pteOutput);
        }


        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        }

        pMainLayout->addLayout(pCharLayout);
        pMainLayout->addLayout(pFormatLayout);
        pMainLayout->addLayout(pOutputLayout);
        pMainLayout->addWidget(m_pButtonBox);
        pMainLayout->setStretchFactor(pOutputLayout,1);
    }
    setLayout(pMainLayout);
}


void TextOutputTestDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)  reject();
}


void TextOutputTestDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void TextOutputTestDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void TextOutputTestDlg::onOutputFont()
{
    if     (m_prbTextFnt->isChecked())  m_pteOutput->setFont(DisplayOptions::textFont());
    else if(m_prbTableFnt->isChecked()) m_pteOutput->setFont(DisplayOptions::tableFont());
    else if(m_prbTreeFnt->isChecked())  m_pteOutput->setFont(DisplayOptions::treeFont());
    m_pteOutput->update();
}


void TextOutputTestDlg::toPlainText()
{
    QString input = m_pptInput->toPlainText();
    m_pteOutput->setPlainText(input);
}


void TextOutputTestDlg::toMarkDown()
{
#if QT_VERSION >= 0x050F00
    QString input = m_pptInput->toPlainText();
    m_pteOutput->setMarkdown(input);
#endif
}


void TextOutputTestDlg::toHTML()
{
    QString input = m_pptInput->toPlainText();
    m_pteOutput->setHtml(input);
}

