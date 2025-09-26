/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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
        PlainTextOutput *m_pptoOutput;
        QDialogButtonBox *m_pButtonBox;

        static bool s_bShell;
        static QByteArray s_WindowGeometry;
};


