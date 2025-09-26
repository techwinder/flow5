/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QPushButton>
#include <QSettings>

#include <xflcore/linestyle.h>


class Curve;
class LineBtn;
class MainFrame;
class XPlane;
class XSail;
class ExponentialSlider;
struct LineStyle;

class CpGraphCtrls : public QWidget
{
    Q_OBJECT

    public:
        CpGraphCtrls(MainFrame*pMainFrame, XPlane *pXPlane, XSail *pXSail);

        void setControls();

        int currentWingIndex() const {return m_pcbWingList->currentIndex();}
        int iStrip() const {return m_iStrip;}
        double spanRelPos() const {return m_SpanRelativePos;}
        double CpSectionScale() const;


    public slots:
        void onCpSectionSlider(int pos);
        void onCpScale(int);
        void onClearCpCurves();

    private:
        void setupLayout();
        void connectSignals();

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private:

        QComboBox *m_pcbWingList;
        QPushButton *m_ppbKeepCpCurve, *m_ppbClearCpCurves;
        QSlider *m_pslCpSectionSlider;
        ExponentialSlider *m_peslScale;
//        QSlider *m_peslScale;

        double m_SpanRelativePos;        /**< Span position for the Cp Graph  */
        int m_iStrip;
        LineStyle m_CpLineStyle;    /**< the style of the lines displayed in the comboboxes*/


        MainFrame *s_pMainFrame;
        XPlane *s_pXPlane;
        XSail *s_pXSail;
};

