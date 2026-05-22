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

        void initDialog(NCollection_List<TopoDS_Shape> const &shapes);

        QSize sizeHint() const override {return QSize(1100, 900);}
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        NCollection_List<TopoDS_Shape> const &shapes() const {return m_Shapes;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();
        void outputMessage(QString const &msg);


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
        NCollection_List<TopoDS_Shape> m_Shapes;

        QDialogButtonBox *m_pButtonBox;

        PlainTextOutput *m_pptoOutput;
        FloatEdit *m_pfePrecision;
        FloatEdit *m_pfeMinTolerance, *m_pfeMaxTolerance;
        QPushButton *m_ppbListShapes;
        QPushButton *m_ppbSmallEdges, *m_ppbFixGaps, *m_ppbFixAll;
        QPushButton *m_ppbReverseShapes, *m_ppbStitch;
        QPushButton *m_ppbClearOutput;

        static double s_Precision, s_MinTolerance, s_MaxTolerance;
        static QByteArray s_Geometry;
};

