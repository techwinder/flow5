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

#pragma once
#include <QCheckBox>

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>

class IntEdit;
class FloatEdit;


class ImageDlg : public QDialog
{
    Q_OBJECT
    public:
        ImageDlg(QWidget *pParent, QVector<double> values, bool bScale, bool bFlipH, bool bFlipV);

        bool bChanged() const {return m_bChanged;}

        QPointF offset();
        double xScale();
        double yScale();

        bool bScaleWithView() const {return m_pchScaleWithView->isChecked();}
        bool bFlipH()         const {return m_pchFlipH->isChecked();}
        bool bFlipV()         const {return m_pchFlipV->isChecked();}

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void connectSignals();

    signals:
        void imageChanged(bool,bool,bool,QPointF,double,double);

    private slots:
        void onButton(QAbstractButton *pButton);
        void onApply();

    private:
        QWidget *m_pParent;

        bool m_bChanged;

        IntEdit *m_pieXOffset, *m_pieYOffset;
        FloatEdit *m_pfeXScale, *m_pfeYScale;
        QCheckBox *m_pchScaleWithView;
        QCheckBox *m_pchFlipH, *m_pchFlipV;
        QDialogButtonBox *m_pButtonBox;
};


