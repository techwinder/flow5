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


#include <QMessageBox>

#include "renamedlg.h"


QByteArray RenameDlg::s_WindowGeometry;

RenameDlg::RenameDlg(QWidget *pParent) : QDialog(pParent)
{
//    setWindowIcon(QIcon(":/icons/f5.png"));
    setWindowTitle("Rename");
    m_strNames.clear();
    setupLayout();
}


void RenameDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)      onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
    else if (m_pOverwriteButton==pButton)                                onOverwrite();
}


void RenameDlg::setupLayout()
{
    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    {
        m_pOverwriteButton = new QPushButton("Overwrite");
        m_pOverwriteButton->setAutoDefault(false);
        m_pButtonBox->addButton(m_pOverwriteButton, QDialogButtonBox::ActionRole);
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_plabMessage = new QLabel("A Message here");

        m_pleName = new QLineEdit("");
        m_pleName->setClearButtonEnabled(true);
        QLabel* NameListLabel = new QLabel("Existing names:");
        m_plwNameList = new QListWidget;
        pMainLayout->setStretchFactor(m_plwNameList, 5);

        pMainLayout->addWidget(m_plabMessage);
        pMainLayout->addWidget(m_pleName);

        pMainLayout->addWidget(NameListLabel);
        pMainLayout->addWidget(m_plwNameList);

        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);

    connect(m_pleName, SIGNAL(textEdited(QString)), this, SLOT(onEnableOverwrite(QString)));
    connect(m_plwNameList, SIGNAL(currentRowChanged(int)), this, SLOT(onSelChangeList(int)));
    connect(m_plwNameList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onDoubleClickList(QListWidgetItem*)));
    //    connect(m_pOverwriteButton, SIGNAL(clicked()), this, SLOT(onOverwrite()));
}


void RenameDlg::initDialog(std::string const &startname, std::vector<std::string> const &existingnames, QString const &question)
{
    QStringList names;
    for(std::string const &name : existingnames)
        names.push_back(QString::fromStdString(name));

    initDialog(QString::fromStdString(startname), names, question);
}


void RenameDlg::initDialog(const QString &startname, QStringList const &existingnames, const QString &question)
{
    m_strNames = existingnames;
    if(question.length()) m_plabMessage->setText(question);
    else                  m_plabMessage->setText("Enter a name");

    m_pleName->setText(startname);
    m_pleName->setFocus();
    m_pleName->selectAll();

    m_plwNameList->clear();
    if(m_strNames.size())
    {
        for (int i=0; i<m_strNames.size(); i++)
        {
            m_plwNameList->addItem(m_strNames.at(i));
        }
    }
    else
    {
        m_plwNameList->setEnabled(false);
        m_pOverwriteButton->setEnabled(false);
    }

    onEnableOverwrite(startname);
}


QString RenameDlg::newName() const
{
    return m_pleName->text().trimmed();
}


void RenameDlg::onEnableOverwrite(QString name)
{
    QList<QListWidgetItem *> itemList = m_plwNameList->findItems(name, Qt::MatchExactly);
    m_pOverwriteButton->setEnabled(itemList.size());
}


void RenameDlg::keyPressEvent(QKeyEvent *pEvent)
{
    // Prevent Return Key from closing App
    switch (pEvent->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    {
        m_pButtonBox->button(QDialogButtonBox::Ok)->setFocus();
        break;
    }
    case Qt::Key_Escape:
    {
        reject();
        return;
    }
    default:
        pEvent->ignore();
    }
}


void RenameDlg::onOverwrite()
{
    done(10);
}


void RenameDlg::onOK()
{
    QString strName = m_pleName->text();
    if (!strName.length())
    {
        QMessageBox::warning(this, "Warning", "Must enter a name");
        m_pleName->setFocus();
        return;
    }

    QString strong;

    //exists?
    for (int l=0; l<m_strNames.size(); l++)
    {
        strong = m_strNames.at(l);
        if(strong == strName)
        {
            QString str = "<p>This operation cannot be undone.<br>Overwrite " + strName + "?</p>";
            if (QMessageBox::Yes == QMessageBox::question(window(), "Question", str,
                                                          QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
            {
                done(10);
                return;
            }
            else return;
        }
    }

    QDialog::accept();
}


void RenameDlg::onSelChangeList(int)
{
    QListWidgetItem *pItem =  m_plwNameList->currentItem();

    if(pItem)
    {
        QString str = pItem->text();
        m_pleName->setText(str);
        m_pleName->selectAll();
        m_pOverwriteButton->setEnabled(true);
    }
}


void RenameDlg::onDoubleClickList(QListWidgetItem * pItem)
{
    if(pItem)
    {
        QString str = pItem->text();
        m_pleName->setText(str);
        m_pOverwriteButton->setEnabled(true);
        onOK();
    }
}


void RenameDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);
}


void RenameDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}


void RenameDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("RenameDlg2");
    {
        s_WindowGeometry = settings.value("WindowGeom", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void RenameDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("RenameDlg2");
    {
        settings.setValue("WindowGeom", s_WindowGeometry);
    }
    settings.endGroup();
}

