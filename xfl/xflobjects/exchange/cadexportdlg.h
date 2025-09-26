/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#pragma once

#include <QDialog>
#include <QSettings>
#include <QListWidget>
#include <QRadioButton>
#include <QLabel>
#include <QString>
#include <QComboBox>
#include <QSplitter>
#include <QDialogButtonBox>

#include <TopoDS_ListOfShape.hxx>
#include <TopoDS_Shape.hxx>

class PlainTextOutput;

class CADExportDlg : public QDialog
{
    Q_OBJECT

    public:
        CADExportDlg(QWidget*pParent);
        void init(TopoDS_Shape const & shape, QString partname);
        void init(TopoDS_ListOfShape const & listofshape, QString partname);

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        QSize sizeHint() const override {return QSize(700, 500);}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    protected:
        void exportBRep();
        void exportIGES();
        void exportSTEP();

        void updateOutput(QString const &strong);

        void makeCommonWts();
        void setupLayout();
        virtual void exportShapes();

    protected slots:
        void onExport();
        void onFormat();
        void onButton(QAbstractButton *pButton);


    protected:
        QListWidget *m_plwListFormat;
        QRadioButton *m_prbBRep, *m_prbIGES, *m_prbSTEP;
        QPushButton *m_ppbExport;
        QDialogButtonBox *m_pButtonBox;
        PlainTextOutput *m_pptoLog;

        QFrame *m_pfrControls;

        TopoDS_ListOfShape m_ShapesToExport;

        QString m_PartName;


        static int s_ExportIndex;
        static QByteArray s_Geometry;
};

