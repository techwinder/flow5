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


#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_ListOfShape.hxx>

#include <QDialog>
#include <QDialogButtonBox>
#include <QRadioButton>

class PlainTextOutput;


class SailCadReaderDlg : public QDialog
{
    Q_OBJECT
    public:
        SailCadReaderDlg(QWidget *pParent);
        void initDialog();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(900,750);}

        TopoDS_ListOfShape const &shapes() const {return m_ListOfShape;}
        bool bShells() const {return s_bShell;}

    private:
        void setupLayout();

    private slots:
        void onButton(QAbstractButton*pButton);
        void onImportFile();
        void accept() override;

    private:
        TopoDS_ListOfShape m_ListOfShape;

        QRadioButton *m_prbFACE, *m_prbSHELL;
        PlainTextOutput *m_ppto;
        QDialogButtonBox *m_pButtonBox;

        static bool s_bShell;
        static QByteArray s_WindowGeometry;
};


