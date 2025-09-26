/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QSettings>
#include <QPlainTextEdit>

class FloatEdit;
class PlainTextOutput;
class PanelCheckDlg : public QDialog
{
    Q_OBJECT

    public:
        PanelCheckDlg(bool bQuads);

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        bool checkSkinny()      const {return s_bCheckSkinny;}
        bool checkMinAngles()   const {return s_bCheckMinAngle;}
        bool checkMinArea()     const {return s_bCheckMinArea;}
        bool checkMinSize()     const {return s_bCheckMinSize;}
        bool checkMinQuadWarp() const {return s_bCheckQuadWarp;}

        void setPanelIndexes(QVector<int> intvalues);
        QVector<int> panelIndexes() const;
        QVector<int> nodeIndexes() const;

        static double qualityFactor() {return s_Quality;}
        static double minAngle()      {return s_MinAngle;}
        static double minArea()       {return s_MinArea;}
        static double minSize()       {return s_MinSize;}
        static double maxQuadWarp()   {return s_MaxQuadWarp;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private slots:
        void accept() override;
        void onCheckSkinny();
        void onCheckMinAngle();
        void onCheckMinArea();
        void onCheckMinSize();
        void onCheckQuadWarp();

    private:
        void setupLayout();

    private:
        FloatEdit *m_pdeQuality, *m_pdeMinAngle, *m_pdeMinArea, *m_pdeMinSize;
        FloatEdit *m_pdeMaxQuadWarp;
        QCheckBox *m_pchCheckSkinny, *m_pchCheckAngle, *m_pchCheckArea, *m_pchCheckSize;
        QCheckBox *m_pchCheckQuadWarp;
        PlainTextOutput *m_pptePanelIndexes, *m_ppteNodeIndexes;

        bool m_bQuads;

        static bool s_bCheckSkinny, s_bCheckMinAngle, s_bCheckMinArea, s_bCheckMinSize, s_bCheckQuadWarp;
        static double s_Quality, s_MinAngle, s_MinArea, s_MinSize, s_MaxQuadWarp;

        static QByteArray s_Geometry;
};

