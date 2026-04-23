/****************************************************************************

    xflwidgets Library
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


