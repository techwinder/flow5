/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QWidget>
#include <QRadioButton>
#include <QSlider>

#include <xfl3d/testgl/gl3dtexture.h>

class IntEdit;
class FloatEdit;

class Attractor2d : public QWidget
{
    Q_OBJECT

    public:
        Attractor2d(QWidget *parent = nullptr);
        ~Attractor2d();

        static QString &lastFileName() {return s_LastFileName;}
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void runAttractor(QWidget *pParent);
        void processImgBlock(int rf, int rl);
        void readParams();
        void updateBtns(bool bStart);
        void paintEvent(QPaintEvent*pEvent) override;
        void customEvent(QEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(1100,900);}

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        double fx(double x, double y) const;
        double fy(double x, double y) const;

    private slots:
        void onBackground();
        void onContinue();
        void onClear();
        void onSaveImg();
        void onOpenImg();
        void updateImg();
        void onTaskFinished();
        void onResizeImage();

    signals:
        void taskFinished();

    private:

        QFrame *m_pFrame;
        QPushButton *m_ppbStart, *m_ppbClear;
        QPushButton *m_ppbSaveImg;
        IntEdit *m_pieWidth, *m_pieHeight;
        FloatEdit *m_pfeTimeOut;
        QLabel *m_plabMaxOcc;
        IntEdit *m_pieMaxOcc;
        FloatEdit *m_pfea, *m_pfeb;
        FloatEdit *m_pfec, *m_pfed;

        QCheckBox *m_pchDark;

        QSlider *m_pslRed, *m_pslGreen, *m_pslBlue;
        QLabel *m_plabInfo;
        QImage *m_pImg;
        QVector<ushort> m_Occupancy;
        QVector<ushort> m_Speed;

        bool m_bIsRunning;
        bool m_bCancel;
        uint m_NSteps;


        static QString s_LastFileName;

        static float s_red, s_green, s_blue;

        static double s_a, s_b;
        static double s_c, s_d;

        static float s_TimeOut;
        static ushort s_MaxOccupancy;

        static bool s_bDark;

        static QSize s_ImgSize;

        static QByteArray s_Geometry;
};


