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

#include <QWidget>
#include <QRadioButton>
#include <QCheckBox>

#include <interfaces/graphs/controls/graphtiles.h>
#include <interfaces/graphs/graph/graph.h>

class MainFrame;
class XDirect;
class XPlane;
class XSail;
class XflDelegate;
class ActionItemModel;
class CPTableView;

class GraphTileCtrls : public QWidget
{
    Q_OBJECT

    public:
        GraphTileCtrls(GraphTiles *pParent=nullptr);
        virtual ~GraphTileCtrls() = default;
        void setGraphTileWt(GraphTiles *pTileWt);

        virtual void setControls();
        virtual void checkGraphActions();

        void setMaxGraphs(int nGraphs);

        virtual void setupLayout();
        virtual void connectSignals();

        void selectActiveVariableSet(int iSet);

        static void setMainFrame(MainFrame *pMainFrame) {s_pMainFrame=pMainFrame;}
        static void setXPlane(XPlane*pXPlane)           {s_pXPlane=pXPlane;}
        static void setXDirect(XDirect*pXDirect)        {s_pXDirect=pXDirect;}


    protected:
        void showEvent(QShowEvent *pEvent) override;
        void fillVariableSetTable();

    protected slots:
        void onCurrentRowChanged(QModelIndex index, QModelIndex);
        void onVarSetClicked(QModelIndex index);
        void onDelete();
        void onInsertAfter();
        void onInsertBefore();
        void onCellChanged(QModelIndex index, QModelIndex);
        void onMoveUp();
        void onMoveDown();

    protected:
        void makeCommonWts();

    protected:
        QRadioButton *m_prbGraph[5];
        QRadioButton *m_prbTwoGraphs,*m_prbFourGraphs, *m_prbAllGraphs;

        CPTableView *m_pcptVariableSet;
        ActionItemModel *m_pVariableSetModel;
        XflDelegate *m_pActionDelegate;

        GraphTiles *m_pGraphTileWt;

    protected:
        static MainFrame *s_pMainFrame;
        static XPlane *s_pXPlane;
        static XDirect *s_pXDirect;

};

