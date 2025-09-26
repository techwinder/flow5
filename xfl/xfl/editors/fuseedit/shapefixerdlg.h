/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QSettings>

#include <TopoDS_Shape.hxx>

class FloatEdit;
class PlainTextOutput;


class ShapeFixerDlg : public QDialog
{
    Q_OBJECT

    public:
        ShapeFixerDlg(QWidget *pParent=nullptr);

        void initDialog(TopoDS_ListOfShape const &shapes);

        QSize sizeHint() const override {return QSize(1100, 900);}
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        TopoDS_ListOfShape const &shapes() const {return m_shapes;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();
        void outputMessage(QString const &msg);
        void listShapeProperties(QString &props, TopoDS_Shape const &shape, QString prefix);


    private slots:
        void onButton(QAbstractButton*pButton);

        void onReadParams();

        void onListShapes();
        void onReverseShapes();
        void onStitchShapes();
        void onSmallEdges();
        void onFixGaps();
        void onFixAll();

    private:
        TopoDS_ListOfShape m_shapes;

        QDialogButtonBox *m_pButtonBox;

        PlainTextOutput *m_pptoOutput;
        FloatEdit *m_pdePrecision;
        FloatEdit *m_pdeMinTolerance, *m_pdeMaxTolerance;
        QPushButton *m_ppbListShapes;
        QPushButton *m_ppbSmallEdges, *m_ppbFixGaps, *m_ppbFixAll;
        QPushButton *m_ppbReverseShapes, *m_ppbStitch;
        QPushButton *m_ppbClearOutput;

        static double s_Precision, s_MinTolerance, s_MaxTolerance;
        static QByteArray s_Geometry;
};

