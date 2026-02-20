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

#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>



#include <interfaces/controls/splinectrl/splinectrl.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/view/section2dwt.h>

#include <api/bspline.h>
#include <api/spline.h>

SplineCtrl::SplineCtrl(QWidget *pParent): QFrame(pParent)
{
    setCursor(Qt::ArrowCursor);

    m_pSpline = nullptr;

    setupLayout();
    connectSignals();
}


void SplineCtrl::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pSplineParamBox = new QGroupBox("Spline definition");
        {
            QVBoxLayout *pSplineParamLayout = new QVBoxLayout;
            {
                m_pcbSplineDegree = new QComboBox;
                m_pcbSplineDegree->clear();
                for (int i=2; i<10; i++)
                {
                    QString str = QString("%1").arg(i);
                    m_pcbSplineDegree->addItem(str);
                }

                m_pcptPoint = new CPTableView(this);
                m_pcptPoint->setEditable(true);
                m_pcptPoint->setWindowTitle("Control points");

                m_pPointModel = new ActionItemModel(this);
                m_pPointModel->setActionColumn(2);
                m_pPointModel->setRowCount(10);//temporary
                m_pPointModel->setColumnCount(3);
                m_pPointModel->setHeaderData(0, Qt::Horizontal, "x");
                m_pPointModel->setHeaderData(1, Qt::Horizontal, "y");
                m_pPointModel->setHeaderData(2, Qt::Horizontal, "Actions");

                m_pcptPoint->setModel(m_pPointModel);
                m_pcptPoint->horizontalHeader()->setStretchLastSection(true);
                m_pPointFloatDelegate = new XflDelegate(this);
                m_pPointFloatDelegate->setActionColumn(2);
                m_pcptPoint->setItemDelegate(m_pPointFloatDelegate);
                m_pPointFloatDelegate->setDigits({5,5,0});
                m_pPointFloatDelegate->setItemTypes({XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::ACTION});

                m_pchClosedTE = new QCheckBox("Force closed TE");
                m_pchClosedTE->setToolTip("Forces the spline's leading and trailing points to be at the same position");
                m_pchSymmetric = new QCheckBox("Symmetric spline");
                QString tip("<p>The trailing end point will be forced in the top half plane, i.e. y>=0.<br>"
                                 "The leading end point will be forced in on the x-axis, i.e. y=0.<br>"
                                 "Make the upper part of the foil with the spline,then Apply to generate "
                                 "the symmetric foil.</p>");
                m_pchSymmetric->setToolTip(tip);

                QHBoxLayout *pDegreeLayout = new QHBoxLayout;
                {
                    QLabel *pLabDegree = new QLabel("Degree");
                    pDegreeLayout->addStretch();
                    pDegreeLayout->addWidget(pLabDegree);
                    pDegreeLayout->addWidget(m_pcbSplineDegree);
                }

                QHBoxLayout *pOptionsLayout = new QHBoxLayout;
                {
                    pOptionsLayout->addWidget(m_pchClosedTE);
                    pOptionsLayout->addStretch();
                    pOptionsLayout->addWidget(m_pchSymmetric);
                }
                pSplineParamLayout->addLayout(pDegreeLayout);
                pSplineParamLayout->addWidget(m_pcptPoint);
                pSplineParamLayout->addLayout(pOptionsLayout);
            }
            m_pSplineParamBox->setLayout(pSplineParamLayout);
        }

        m_pBunchBox = new QGroupBox("Output points bunching");
        {
            QGridLayout *pBunchLayout = new QGridLayout;
            {
                QLabel *pLabNoAmp = new QLabel("None");
                pLabNoAmp->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
                QLabel *pLabAmp = new QLabel("Full");
                pLabAmp->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
                m_pslBunchAmp = new QSlider(Qt::Horizontal);
                m_pslBunchAmp->setRange(0, 100);
                m_pslBunchAmp->setTickPosition(QSlider::TicksBelow);
                QString tip = "Defines the intensity of the bunching of the output points";
                pLabNoAmp->setToolTip(tip);
                pLabAmp->setToolTip(tip);
                m_pslBunchAmp->setToolTip(tip);

                pBunchLayout->addWidget(pLabNoAmp,     1,1);
                pBunchLayout->addWidget(pLabAmp,       1,3);
                pBunchLayout->addWidget(m_pslBunchAmp, 1,2);
            }
            m_pBunchBox->setLayout(pBunchLayout);
        }

        m_pOuputFrame = new QGroupBox("Display");
        {
            QGridLayout *pOutputLayout = new QGridLayout;
            {
                m_pchShow        = new QCheckBox("Show");
                m_pchShowNormals = new QCheckBox("Normals");
                QLabel *pLabSplineStyle = new QLabel("Style:");
                m_plbSplineStyle = new LineBtn;

                QLabel *pLabN = new QLabel("Number of curve points:");
                m_pieOutputPoints = new IntEdit;
                m_pieOutputPoints->setMin(0);
                m_pieOutputPoints->setMax(1000);

                pOutputLayout->addWidget(m_pchShow,         1,2);
                pOutputLayout->addWidget(m_pchShowNormals,  2,2);
                pOutputLayout->addWidget(pLabSplineStyle,   3,1, Qt::AlignRight | Qt::AlignVCenter);
                pOutputLayout->addWidget(m_plbSplineStyle,  3,2);
                pOutputLayout->addWidget(pLabN,             4,1, Qt::AlignRight | Qt::AlignVCenter);
                pOutputLayout->addWidget(m_pieOutputPoints, 4,2);
                pOutputLayout->setColumnStretch(1,3);
            }

            m_pOuputFrame->setLayout(pOutputLayout);
        }

        pMainLayout->addWidget(m_pSplineParamBox);
        pMainLayout->addWidget(m_pBunchBox);
        pMainLayout->addWidget(m_pOuputFrame);

        setLayout(pMainLayout);
    }
}


void SplineCtrl::initSplineCtrls(Spline *pSpline)
{
    if(!pSpline) return;
    m_pSpline = pSpline;

    m_plbSplineStyle->setTheStyle(m_pSpline->theStyle());

    m_pcbSplineDegree->setEnabled(m_pSpline->isBSpline());

    m_pchSymmetric->setChecked(pSpline->issymmetric());

    if(m_pSpline->isBSpline())
    {
        BSpline *pBS = dynamic_cast<BSpline*>(m_pSpline);
        m_pcbSplineDegree->setCurrentIndex(pBS->degree()-2);
        m_pcbSplineDegree->setEnabled(true);
    }
    else
    {
        m_pcbSplineDegree->setEnabled(false);
    }

    m_pieOutputPoints->setValue(m_pSpline->outputSize());

    m_pchShow->setChecked(m_pSpline->isVisible());
    m_pchShowNormals->setChecked(m_pSpline->bShowNormals());

    m_pchClosedTE->setChecked(m_pSpline->isClosed());

    int vamp = int(pSpline->bunchAmplitude()*100.0);
    m_pslBunchAmp->setValue(vamp);


    fillPointModel();
}


void SplineCtrl::connectSignals()
{
    connect(m_pcptPoint, SIGNAL(clicked(QModelIndex)), SLOT(onCtrlPointTableClicked(QModelIndex)));
    connect(m_pcptPoint, SIGNAL(dataPasted()), SLOT(onUpdate()));
    connect(m_pcptPoint->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));

    connect(m_pPointFloatDelegate, SIGNAL(closeEditor(QWidget*)),  SLOT(onUpdate()));
    connect(m_pslBunchAmp,         SIGNAL(sliderMoved(int)),       SLOT(onBunchSlide(int)));
    connect(m_plbSplineStyle,      SIGNAL(clickedLB(LineStyle)),   SLOT(onSplineStyle(LineStyle)));
    connect(m_pcbSplineDegree,     SIGNAL(activated(int)),         SLOT(onUpdate()));
    connect(m_pieOutputPoints,     SIGNAL(intChanged(int)),        SLOT(onUpdate()));
    connect(m_pchShow,             SIGNAL(clicked(bool)),          SLOT(onUpdate()));
    connect(m_pchShowNormals,      SIGNAL(clicked(bool)),          SLOT(onUpdate()));
    connect(m_pchClosedTE,         SIGNAL(clicked(bool)),          SLOT(onClosedTE()));
    connect(m_pchSymmetric,        SIGNAL(clicked(bool)),          SLOT(onForcesymmetric()));
}


void SplineCtrl::showPointTable(bool bShow)
{
    m_pcptPoint->setVisible(bShow);
}


void SplineCtrl::onForcesymmetric()
{
    if(m_pSpline)
    {
        m_pSpline->setForcedsymmetric(m_pchSymmetric->isChecked());
        m_pchClosedTE->setChecked(false);
        emit splineChanged();
    }
}


void SplineCtrl::onClosedTE()
{
    if(m_pSpline)
    {
        m_pSpline->setClosed(m_pchClosedTE->isChecked());
        m_pchSymmetric->setChecked(false);
        emit splineChanged();
    }
}


void SplineCtrl::onBunchSlide(int)
{
    int val0 = m_pslBunchAmp->value();
    double amp = double(val0)/100.0; // k=0.0 --> uniform weight, k=1-->full varying weights;

    m_pSpline->setBunchAmplitude(amp);

    updateSplines();
}


void SplineCtrl::showEvent(QShowEvent *pEvent)
{
    resizeColumns();
    pEvent->accept();
}

void SplineCtrl::hideEvent(QHideEvent *pEvent)
{
    QWidget::hideEvent(pEvent);
}


void SplineCtrl::resizeEvent(QResizeEvent *pEvent)
{
    if(!m_pPointModel || !m_pcptPoint) return;

    resizeColumns();

    pEvent->accept();
}


void SplineCtrl::resizeColumns()
{
    int n = m_pPointModel->actionColumn();
    QHeaderView *pHHeader = m_pcptPoint->horizontalHeader();
    //pHHeader->setDefaultSectionSize(1);
    pHHeader->setSectionResizeMode(n, QHeaderView::Stretch);
    pHHeader->resizeSection(n, 1);


    double w = double(m_pcptPoint->width())*(85.0/100.0);
    int w01 = int(w*0.45);

    m_pcptPoint->setColumnWidth(0,w01);
    m_pcptPoint->setColumnWidth(1,w01);
}


void SplineCtrl::fillPointModel()
{
    if(!m_pSpline) return;
    m_pPointModel->setRowCount(m_pSpline->ctrlPointCount());
    for (int i=0; i<m_pSpline->ctrlPointCount(); i++)
    {
        QModelIndex Xindex = m_pPointModel->index(i, 0, QModelIndex());
        m_pPointModel->setData(Xindex,m_pSpline->controlPoint(i).x);

        QModelIndex Zindex = m_pPointModel->index(i, 1, QModelIndex());
        m_pPointModel->setData(Zindex, m_pSpline->controlPoint(i).y);
    }
}


void SplineCtrl::readData()
{
    if(!m_pSpline) return;

    for(int i=0; i<m_pSpline->ctrlPointCount(); i++)
    {
        QModelIndex index = m_pPointModel->index(i, 0, QModelIndex());
        double x = index.data().toDouble();

        index = m_pPointModel->index(i, 1, QModelIndex());
        double y = index.data().toDouble();

        m_pSpline->setCtrlPoint(i,x,y);
    }

    int ideg = m_pcbSplineDegree->currentIndex()+2;
    if(m_pSpline->ctrlPointCount())
    {
        if(ideg<m_pSpline->ctrlPointCount())
        {
            // there are enough control points for this degree
            m_pSpline->setDegree(ideg);
        }
        else
        {
            // too few control points, adjust the degree
            QMessageBox::warning(this, tr("Warning"), tr("The spline degree must be less than the number of control points"));
            m_pSpline->setDegree(std::max(2,int(m_pSpline->ctrlPointCount())-1));
            m_pcbSplineDegree->setCurrentIndex(m_pSpline->degree()-2);
        }
    }

    m_pSpline->setOutputSize(m_pieOutputPoints->value());
    m_pSpline->setVisible(m_pchShow->isChecked());
    m_pSpline->showNormals(m_pchShowNormals->isChecked());

    m_pSpline->setForcedsymmetric(m_pchSymmetric->isChecked());
    m_pSpline->setClosed(m_pchClosedTE->isChecked());
}


void SplineCtrl::onUpdate()
{
    readData();
    updateSplines();
}


void SplineCtrl::updateSplines()
{
    if(m_pSpline)
    {
        m_pSpline->updateSpline();
        m_pSpline->makeCurve();
        emit splineChanged();
    }
}


void SplineCtrl::onDelete()
{
    QModelIndex index = m_pcptPoint->currentIndex();
    int sel = index.row();

    if(sel<0 || sel>=int(m_pSpline->ctrlPointCount())) return;

    m_pSpline->removeCtrlPoint(sel);

    fillPointModel();

    onUpdate();
}


void SplineCtrl::onInsertBefore()
{
    int sel = m_pcptPoint->currentIndex().row();

    Vector2d newpt;
    if(m_pSpline->ctrlPointCount()==0)
    {
        // (0,0) is a fine starting point
    }
    else if(sel==0)
    {
        newpt.x = m_pSpline->controlPoint(sel).x*1.1;
        newpt.y = m_pSpline->controlPoint(sel).y*1.1;
    }
    else   newpt = (m_pSpline->controlPoint(sel-1)+m_pSpline->controlPoint(sel))/2.0;
/*    else  if(sel==m_pSpline->controlPointCount()-1)
    {
        newpt.x = m_pSpline->controlPoint(sel).x*1.1;
        newpt.y = m_pSpline->controlPoint(sel).y*1.1;
    }*/

    m_pSpline->insertCtrlPointAt(sel, newpt);

    fillPointModel();

    QModelIndex index = m_pPointModel->index(sel, 0, QModelIndex());
    m_pcptPoint->setCurrentIndex(index);
    m_pcptPoint->selectRow(index.row());

    onUpdate();
}


void SplineCtrl::onInsertAfter()
{
    int sel = m_pcptPoint->currentIndex().row();

    Vector2d newpt;
    if(m_pSpline->ctrlPointCount()==0)
    {
        // (0,0) is a fine starting point
    }
    else if(sel==int(m_pSpline->ctrlPointCount())-1)
    {
        // addresses size=1 also
        newpt.x = m_pSpline->controlPoint(sel).x*1.1;
        newpt.y = m_pSpline->controlPoint(sel).y*1.1;
    }
    else   newpt = (m_pSpline->controlPoint(sel)+m_pSpline->controlPoint(sel+1))/2.0;

    m_pSpline->insertCtrlPointAt(sel+1, newpt);

    fillPointModel();

    QModelIndex index = m_pPointModel->index(sel+1, 0, QModelIndex());
    m_pcptPoint->setCurrentIndex(index);
    m_pcptPoint->selectRow(index.row());

    onUpdate();
}


void SplineCtrl::onCurrentRowChanged(QModelIndex index, QModelIndex)
{
    if(m_pSpline)
    {
        m_pSpline->setSelectedPoint(index.row());
        emit pointSelChanged();
    }
}


void SplineCtrl::onCtrlPointTableClicked(QModelIndex index)
{
    if(!index.isValid())
    {
    }
    else
    {
        if(m_pSpline)
        {
            m_pSpline->setSelectedPoint(index.row());
            emit pointSelChanged();
        }

        if(index.column()==m_pPointModel->actionColumn())
        {
            m_pcptPoint->selectRow(index.row());
            QRect itemrect = m_pcptPoint->visualRect(index);
            QPoint menupos = m_pcptPoint->mapToGlobal(itemrect.topLeft());
            QMenu *pWingTableRowMenu = new QMenu("Section",this);

            QAction *m_pInsertBeforeAct    = new QAction("Insert before", this);
            QAction *m_pInsertAfterAct    = new QAction("Insert after", this);
            QAction *m_pDeleteAct        = new QAction("Delete", this);

            connect(m_pDeleteAct,       SIGNAL(triggered(bool)), SLOT(onDelete()));
            connect(m_pInsertBeforeAct, SIGNAL(triggered(bool)), SLOT(onInsertBefore()));
            connect(m_pInsertAfterAct,  SIGNAL(triggered(bool)), SLOT(onInsertAfter()));

            pWingTableRowMenu->addAction(m_pInsertBeforeAct);
            pWingTableRowMenu->addAction(m_pInsertAfterAct);
            pWingTableRowMenu->addAction(m_pDeleteAct);
            pWingTableRowMenu->exec(menupos, m_pInsertBeforeAct);
        }
    }
}


/**
 * The user has changed the color of the current curve
 */
void SplineCtrl::onSplineStyle(LineStyle)
{
    LineMenu *m_pLineMenu = new LineMenu(nullptr, true);
    m_pLineMenu->initMenu(m_pSpline->stipple(), m_pSpline->width(), m_pSpline->color(), m_pSpline->pointStyle());
    m_pLineMenu->exec(QCursor::pos());
    LineStyle ls = m_pLineMenu->theStyle();
    m_plbSplineStyle->setTheStyle(ls);
    m_pSpline->setTheStyle(ls);

    emit splineChanged();
}


