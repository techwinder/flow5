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


#include <TopExp_Explorer.hxx>
#include <TopoDS_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>




#include <QVBoxLayout>


#include "shapedlg.h"

#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dshapesview.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <api/occ_globals.h>

Quaternion ShapeDlg::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);
QByteArray ShapeDlg::s_HSplitterSizes;
QByteArray ShapeDlg::s_VSplitterSizes;
QByteArray ShapeDlg::s_Geometry;


ShapeDlg::ShapeDlg(QWidget *pParent) : QDialog(pParent)
{
    setupLayout();
    connectSignals();
}


void ShapeDlg::setupLayout()
{
    m_pHSplitter = new QSplitter(Qt::Horizontal);
    {
        m_pHSplitter->setChildrenCollapsible(false);

        QFrame *pLeftFrame = new QFrame;
        {
            QVBoxLayout *pLeftLayout = new QVBoxLayout;
            {
                m_plwShapes  = new QListWidget;
                m_ppbDelete  = new QPushButton(tr("Delete selected"));
                m_pptoOutput = new PlainTextOutput;
                m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
                {
                    QPushButton *ppbClearOutput = new QPushButton(tr("Clear output"));
                    connect(ppbClearOutput, SIGNAL(clicked(bool)), m_pptoOutput, SLOT(clear()));
                    m_pButtonBox->addButton(ppbClearOutput, QDialogButtonBox::ActionRole);

                    connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
                }
                pLeftLayout->addWidget(m_plwShapes);
                pLeftLayout->addWidget(m_ppbDelete);
                pLeftLayout->addWidget(m_pptoOutput);
                pLeftLayout->addWidget(m_pButtonBox);
            }
            pLeftFrame->setLayout(pLeftLayout);
        }

        QFrame *p3dViewFrame = new QFrame;
        {
            QVBoxLayout *pTWPageLayout = new QVBoxLayout;
            {
                m_pglShapesView = new gl3dShapesView;
                m_pglControls  = new gl3dGeomControls(m_pglShapesView, Qt::Horizontal, true);
                pTWPageLayout->addWidget(m_pglShapesView);
                pTWPageLayout->addWidget(m_pglControls);
            }
            p3dViewFrame->setLayout(pTWPageLayout);
        }

        m_pHSplitter->addWidget(pLeftFrame);
        m_pHSplitter->addWidget(p3dViewFrame);
    }
    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);
}


void ShapeDlg::connectSignals()
{
    connect(m_plwShapes, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(onShellChanged()));
    connect(m_ppbDelete, SIGNAL(clicked()),                      SLOT(onDeleteShell()));
}


void ShapeDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)       accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)    reject();
}


void ShapeDlg::initDialog(TopoDS_ListOfShape const &shapes)
{
    fillShapes(shapes);
    m_pglShapesView->setShapes(m_Shapes);
    m_pglShapesView->update();
}


void ShapeDlg::showEvent(QShowEvent *pEvent)
{
    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0)  m_pHSplitter->restoreState(s_HSplitterSizes);
//    if(s_VSplitterSizes.length()>0)  m_pVSplitter->restoreState(s_VSplitterSizes);


    m_pglShapesView->restoreViewPoint(s_ab_quat);

    QDialog::showEvent(pEvent);
}


void ShapeDlg::hideEvent(QHideEvent *pEvent)
{
    s_Geometry = saveGeometry();
    s_HSplitterSizes = m_pHSplitter->saveState();
//    s_VSplitterSizes = m_pVSplitter->saveState();


    m_pglShapesView->saveViewPoint(s_ab_quat);

    QDialog::hideEvent(pEvent);
}


void ShapeDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("ShapeDlg");
    {
        s_Geometry = settings.value("Geometry", QByteArray()).toByteArray();
        s_HSplitterSizes = settings.value("HSplitterSizes").toByteArray();
        s_VSplitterSizes = settings.value("VSplitterSizes").toByteArray();

   }
    settings.endGroup();
}


void ShapeDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("ShapeDlg");
    {
        settings.setValue("Geometry", s_Geometry);
        settings.setValue("HSplitterSizes", s_HSplitterSizes);
        settings.setValue("VSplitterSizes", s_VSplitterSizes);
    }
    settings.endGroup();
}


void ShapeDlg::fillShapes(TopoDS_ListOfShape const &shapes)
{
    m_plwShapes->clear();
    int iShell=0;
    for(TopTools_ListIteratorOfListOfShape shapeit(shapes); shapeit.More(); shapeit.Next())
    {
        TopExp_Explorer shapeExplorer;
        for (shapeExplorer.Init(shapeit.Value(),TopAbs_SHELL); shapeExplorer.More(); shapeExplorer.Next())
        {
            try
            {
                TopoDS_Shell aShell = TopoDS::Shell(shapeExplorer.Current());

                m_Shapes.append(aShell);
                m_plwShapes->addItem(QString::asprintf("Shell_%d", iShell));

                iShell++;
            }

            catch(Standard_TypeMismatch const &)
            {
            }
        }
    }
}


void ShapeDlg::onShellChanged()
{
    QModelIndex idx = m_plwShapes->currentIndex();
    if(idx.isValid())
    {
        m_pglShapesView->clearHighlighted();
        m_pglShapesView->setHighlighted(idx.row());
        m_pglShapesView->update();
        std::string strange, prefix;
        occ::listShapeContent(m_Shapes.at(idx.row()), strange, prefix);
        m_pptoOutput->onAppendStdText(strange+"\n");
    }
}


void ShapeDlg::onDeleteShell()
{
    QModelIndex idx = m_plwShapes->currentIndex();
    if(idx.isValid())
    {
        m_Shapes.removeAt(idx.row());
        m_pglShapesView->resetShapes();
        m_pglShapesView->update();
    }
}


