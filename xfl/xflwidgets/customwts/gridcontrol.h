/****************************************************************************

    xflwidgets Library
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QCheckBox>


class LineBtn;
class Grid;

class GridControl : public QWidget
{
    friend class GridSettingsDlg;

    Q_OBJECT

    public:
        GridControl(QWidget *pParent = nullptr);

        void initControls(Grid *pGrid);
        void setupLayout();
        void connectSignals();


    signals:
        void gridModified(bool);

    private slots:
        void onXAxisStyle();
        void onYAxisStyle();
        void onXMajStyle();
        void onXMinStyle();
        void onYMajStyle();
        void onYMinStyle();
        void onXAxisShow(bool bShow);
        void onYAxisShow(bool bShow);
        void onXMajShow(bool bShow);
        void onYMajShow(bool bShow);
        void onXMinShow(bool bShow);
        void onYMinShow(bool bShow);

    private:
        Grid *m_pGrid;

        QCheckBox  *m_pchXAxisShow, *m_pchYAxisShow, *m_pchXMajShow, *m_pchYMajShow, *m_pchXMinShow, *m_pchYMinShow;
        LineBtn *m_plbXAxisStyle, *m_plbYAxisStyle, *m_plbXMajStyle, *m_plbYMajStyle, *m_plbXMinStyle, *m_plbYMinStyle;

};


