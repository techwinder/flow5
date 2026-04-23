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

#include <QTreeView>
#include <QAction>
#include <QLineEdit>


class CrossCheckBox;

class ExpandableTreeView : public QTreeView
{
    Q_OBJECT

        friend class FoilExplorer;
        friend class PlaneExplorer;
        friend class BoatExplorer;

    public:
        ExpandableTreeView(QWidget *pParent = nullptr);

        void setOverallCheckedState(Qt::CheckState state);
        QWidget * cmdWidget() {return m_pfrControls;}

        int sizeHintForColumn(int column) const override;
        QSize sizeHint() const override;
//        void resizeEvent(QResizeEvent *pEvent) override;

        void enableSelectBox(bool bEnable);

        QString filter() const {return m_pleFilter->text();}

    private:
        void initETV();

    public slots:
        void onObjectLevel();
        void onPolarLevel();
        void onOpPointLevel();
        void onLevelMinus();
        void onLevelPlus();
        void onHideShowAll(bool bChecked);

    signals:
        void switchAll(bool);

    private:
        QLineEdit *m_pleFilter;

        QAction *m_pLevel0Action, *m_pLevel1Action, *m_pLevel2Action, *m_pLevelPlus, *m_pLevelMinus;
        QAction *m_pCollapseAll, *m_pExpandAll;
        CrossCheckBox *m_pchHideShowAll;

        QFrame *m_pfrControls;

};

