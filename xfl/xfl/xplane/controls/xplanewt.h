/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QWidget>
#include <QSettings>
#include <QSplitter>

class XPlane;
class gl3dXPlaneView;
class gl3dGeomControls;
class POpp3dCtrls;

class XPlaneWt : public QWidget
{
    public:
        XPlaneWt(XPlane *pXPlane, gl3dXPlaneView *pgl3dXPlaneView);

        void setControls();
        void updateView();
        void updateObjectData();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        gl3dXPlaneView *gl3dFloatView() {return m_pgl3dXPlaneFloatView;}

    private:
        void setupLayout();

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private:
        gl3dXPlaneView *m_pgl3dXPlaneFloatView;              /**< a pointer to the floating widget for 3d rendering */
        POpp3dCtrls *m_pPOpp3dControls;

        QSplitter *m_pHSplitter;

    private:
        static QByteArray s_Geometry;
        static QByteArray s_HSplitterSizes;
        XPlane *m_pXPlane;
};

