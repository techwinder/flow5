
/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QDebug>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QFileDialog>


#include "foil1splinedlg.h"
#include <xflfoil/editors/foilwt.h>
#include <xflfoil/splinectrl/splineapproxdlg.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/foil.h>
#include <xflgeom/geom2d/splines/spline.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customdlg/intvaluesdlg.h>
#include <xflwidgets/line/linebtn.h>
#include <xflwidgets/line/linemenu.h>

QByteArray Foil1SplineDlg::s_HSplitterSizes;
bool Foil1SplineDlg::s_bCubicSpline = false;
CubicSpline Foil1SplineDlg::s_CS;
BSpline Foil1SplineDlg::s_BS;


Foil1SplineDlg::Foil1SplineDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle("Foil from spline");
    setupLayout();
    connectSignals();

    if(s_BS.ctrlPointCount()==0)
    {
        s_BS.resetSpline();
        s_BS.makeDefaultControlPoints(m_pchClosedTE->isChecked(), false);
        s_BS.updateSpline();
        s_BS.makeCurve();
        s_BS.setColor(Qt::darkYellow);
    }

    s_CS.setBunchType(Spline::DOUBLESIG);
    if(s_CS.ctrlPointCount()==0)
    {
        s_CS.resetSpline();
        s_CS.makeDefaultControlPoints(m_pchClosedTE->isChecked(), false);
        s_CS.updateSpline();
        s_CS.makeCurve();
        s_CS.setColor(Qt::cyan);
    }
}


void Foil1SplineDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);

    m_pFoilWt->setBufferFoil(nullptr);
    m_pBufferFoil->setVisible(false);
    m_pBufferFoil->setName("Splined foil");

    m_BSplineStack.clear();
    m_BSplineStack.push(s_BS);
    m_BStackPos = 0;

    m_CSplineStack.clear();
    m_CSplineStack.push(s_CS);
    m_CStackPos = 0;

    //clean up past mistakes
    if(s_BS.outputSize()<=0) s_BS.setOutputSize(79);
    if(s_CS.outputSize()<=0) s_CS.setOutputSize(79);

    if(s_bCubicSpline)
    {
         s_CS.setVisible(true);
         s_BS.setVisible(false);
         m_pFoilWt->setSpline(&s_CS);
         fillCtrls(&s_CS);
    }
    else
    {
         s_BS.setVisible(true);
         s_CS.setVisible(false);
         m_pFoilWt->setSpline(&s_BS);
         fillCtrls(&s_BS);
    }

    if(!m_pRefFoil) onApply();


    m_prbCS->setChecked(s_bCubicSpline);
    m_prbBS->setChecked(!s_bCubicSpline);

}


void Foil1SplineDlg::onApply()
{
    const Spline *pSpline = nullptr;
    if(s_bCubicSpline) pSpline = &s_CS;
    else               pSpline = &s_BS;

    if(!pSpline->issymmetric())
    {
        int npts = pSpline->outputSize();
        m_pBufferFoil->m_BaseNode.resize(npts);
        for(int i=0; i<npts; i++)
        {
            m_pBufferFoil->m_BaseNode[i] = pSpline->outputPt(i);
        }
    }
    else
    {
        int npts = pSpline->outputSize();
        m_pBufferFoil->m_BaseNode.resize(npts*2-1);
        for(int i=0; i<npts; i++)
        {
            m_pBufferFoil->m_BaseNode[i] = pSpline->outputPt(i);
        }

        //make symmetric half
        int j = npts;
        for(int i=npts-2; i>=0; i--)
        {
            m_pBufferFoil->m_BaseNode[j] = pSpline->outputPt(i);
            j++;
        }
    }

    m_pBufferFoil->initGeometry();

    m_pFoilWt->update();

}


void Foil1SplineDlg::onSplineType()
{
    s_bCubicSpline = m_prbCS->isChecked();
    s_CS.setVisible(s_bCubicSpline);
    s_BS.setVisible(!s_bCubicSpline);

    if(s_bCubicSpline)
    {
        m_pFoilWt->setSpline(&s_CS);
        fillCtrls(&s_CS);
    }
    else
    {
        m_pFoilWt->setSpline(&s_BS);
        fillCtrls(&s_BS);
    }
    m_pFoilWt->update();
}


void Foil1SplineDlg::onMakeDefaultSpline()
{
    Spline *pSpline(nullptr);
    if(s_bCubicSpline)
    {
        s_BS.setVisible(false);
        s_CS.setVisible(true);
        pSpline = &s_CS;
        clearCStack(0);
    }
    else
    {
        s_BS.setVisible(true);
        s_CS.setVisible(false);
        pSpline = &s_BS;

        int ideg = m_pcbSplineDegree->currentIndex()+2;
        pSpline->setDegree(ideg);

        clearBStack(0);
    }


    int npts = 7;
    pSpline->setControlSize(npts);

    for(int i=0; i<npts; i++)
    {
        double t = double(i)/double(npts-1);
        double x = (1.0+cos(2.0*PI*t))/2.0;
        double y =      sin(2.0*PI*t) *0.1;
        if(i==0)      y =  0.005;
        if(i==npts-1) y = -0.005;
        pSpline->setCtrlPoint(i,x,y);
        pSpline->setWeight(i,1.0);
    }

    pSpline->setOutputSize(m_pieNPanels->value());
    pSpline->updateSpline();
    pSpline->makeCurve();


    m_pFoilWt->setSpline(pSpline);
    fillCtrls(pSpline);
    update();
}


void Foil1SplineDlg::onReset()
{
    m_pBufferFoil->clearPointArrays();
    m_pRefFoil = nullptr;
//    onNewSpline();
    update();
}


void Foil1SplineDlg::setupLayout()
{
    QFrame *pLeftFrame = new QFrame;
    {
        pLeftFrame->setCursor(Qt::ArrowCursor);
        QVBoxLayout *pLeftLayout = new QVBoxLayout;
        {

            QGroupBox *pTypeBox = new QGroupBox("Spline type");
            {
                QVBoxLayout *pTypeLayout = new QVBoxLayout;
                {
                    QHBoxLayout *pSplineLayout = new QHBoxLayout;
                    {
                        m_prbBS = new QRadioButton("B-Spline");
                        m_prbCS = new QRadioButton("Cubic Spline");

                        pSplineLayout->addStretch();
                        pSplineLayout->addWidget(m_prbBS);
                        pSplineLayout->addStretch();
                        pSplineLayout->addWidget(m_prbCS);
                        pSplineLayout->addStretch();
                    }

                    QHBoxLayout * pStyleLayout = new QHBoxLayout;
                    {
                        QLabel *pLabSplineStyle = new QLabel("Style:");
                        m_plbSplineStyle = new LineBtn;

                        pStyleLayout->addWidget(pLabSplineStyle);
                        pStyleLayout->addWidget(m_plbSplineStyle);
                        pStyleLayout->addStretch();
                    }

                    QGridLayout *pPropsLayout = new QGridLayout;
                    {
                        QLabel *pLabDeg = new QLabel("Degree=");
                        m_pcbSplineDegree = new QComboBox;
                        for (int i=2; i<10; i++)
                        {
                            QString str = QString::asprintf("%7d",i);
                            m_pcbSplineDegree->addItem(str);
                        }

                        m_pchClosedTE = new QCheckBox("Force closed TE");
                        m_pchClosedTE->setToolTip("Forces the spline's leading and trailing points to be at the same position");


                        pPropsLayout->addWidget(pLabDeg,           1,1);
                        pPropsLayout->addWidget(m_pcbSplineDegree, 1,2);

                        pPropsLayout->addWidget(m_pchClosedTE,     4,1,1,2);

                        pPropsLayout->setColumnStretch(3,1);
                    }

                    pTypeLayout->addLayout(pSplineLayout);
                    pTypeLayout->addLayout(pStyleLayout);
                    pTypeLayout->addLayout(pPropsLayout);
                }
                pTypeBox->setLayout(pTypeLayout);
            }

            QGroupBox *m_pBunchBox = new QGroupBox("Node bunching");
            {
                QVBoxLayout *pBoxLayout = new QVBoxLayout;
                {
                    QHBoxLayout *pNbLayout = new QHBoxLayout;
                    {
                        QLabel *pLabNPanels = new QLabel("Number of panels");
                        pLabNPanels->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                        pLabNPanels->setPalette(m_Palette);
                        pLabNPanels->setAttribute(Qt::WA_NoSystemBackground);
                        m_pieNPanels = new IntEdit;
                        QString tip= "CAUTION: XFoil does not accept number of panel greater than ~300\n"
                                     "Adjust the number of panels and the bunching parameters to\n"
                                     "achieve the desired point distribution.";
                        m_pieNPanels->setToolTip(tip);

                        pNbLayout->addWidget(pLabNPanels);
                        pNbLayout->addWidget(m_pieNPanels);
                        pNbLayout->addStretch();
                    }

                    QHBoxLayout *pBunchLayout = new QHBoxLayout;
                    {
                        QLabel *pLabNoAmp = new QLabel("Uniform");
                        pLabNoAmp->setPalette(m_Palette);
                        pLabNoAmp->setAttribute(Qt::WA_NoSystemBackground);
                        pLabNoAmp->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
                        QLabel *pLabAmp = new QLabel("Bunched");
                        pLabAmp->setPalette(m_Palette);
                        pLabAmp->setAttribute(Qt::WA_NoSystemBackground);
                        pLabAmp->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
                        m_pslBunchAmp = new QSlider(Qt::Horizontal);
                        m_pslBunchAmp->setPalette(m_SliderPalette);
                        m_pslBunchAmp->setRange(0, 100);
                        m_pslBunchAmp->setTickInterval(5);
//                        m_pslBunchAmp->setMinimumWidth(s_TableFontStruct.averageCharWidth()*50);
                        m_pslBunchAmp->setTickPosition(QSlider::TicksBelow);
                        m_pslBunchAmp->setAutoFillBackground(true);

                        QString tip = "Use the sliders to bunch the panels.\n"
                                 "The amplitude slider determines the intensity of the bunching.\n"
                                 "The distribution slider determines whether the bunching is towards\n"
                                 "the leading or the trailing edge.";
                        m_pslBunchAmp->setToolTip(tip);


                        pBunchLayout->addWidget(pLabNoAmp);
                        pBunchLayout->addWidget(m_pslBunchAmp);
                        pBunchLayout->addWidget(pLabAmp);
                        pBunchLayout->setStretchFactor(m_pslBunchAmp,1);

                    }

                    pBoxLayout->addLayout(pNbLayout);
                    pBoxLayout->addLayout(pBunchLayout);
                }
                m_pBunchBox->setLayout(pBoxLayout);
            }


            QGroupBox *pOutputBox = new QGroupBox("Display");
            {
                QGridLayout *pOutputLayout = new QGridLayout;
                {
                    m_pchShowCtrlPts = new QCheckBox("Control points");
                    m_pchShowNormals = new QCheckBox("Normals");


                    pOutputLayout->addWidget(m_pchShowCtrlPts,  1,2);
                    pOutputLayout->addWidget(m_pchShowNormals,  2,2);

                    pOutputLayout->setColumnStretch(1,3);
                    pOutputBox->setLayout(pOutputLayout);
                }

            }

            QHBoxLayout *pUndoRedoLayout = new QHBoxLayout;
            {
                m_ppbUndo = new QPushButton(QIcon(":/icons/OnUndo.png"), "Undo");
                m_ppbUndo->setToolTip("Ctrl+Z");
                m_ppbRedo = new QPushButton(QIcon(":/icons/OnRedo.png"), "Redo");
                m_ppbRedo->setToolTip("Ctrl+Y, Ctrl+Shift+Z");
                pUndoRedoLayout->addWidget(m_ppbUndo);
                pUndoRedoLayout->addWidget(m_ppbRedo);
            }


            m_ppbActionsMenuBtn = new QPushButton("Actions");
            {
                QMenu *pActionsMenu = new QMenu("Actions");
                m_ppbActionsMenuBtn->setMenu(pActionsMenu);
            }

            pLeftLayout->addWidget(pTypeBox);
            pLeftLayout->addWidget(m_pBunchBox);
            pLeftLayout->addStretch();
            pLeftLayout->addWidget(pOutputBox);
            pLeftLayout->addLayout(pUndoRedoLayout);
            pLeftLayout->addWidget(m_ppbActionsMenuBtn);
        }
        pLeftFrame->setLayout(pLeftLayout);
    }

    m_pHSplitter = new QSplitter;
    {
        m_pHSplitter->setChildrenCollapsible(false);
        m_pHSplitter->addWidget(pLeftFrame);
        m_pHSplitter->addWidget(m_pFoilWt);
        m_pHSplitter->setStretchFactor(0, 1);
        m_pHSplitter->setStretchFactor(1, 5);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(m_pHSplitter);
    setLayout(pMainLayout);
}


void Foil1SplineDlg::connectSignals()
{
    connect(m_pHSplitter,          SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));

    connect(m_ppbUndo,             SIGNAL(clicked(bool)),          SLOT(onUndo()));
    connect(m_ppbRedo,             SIGNAL(clicked(bool)),          SLOT(onRedo()));

    connect(m_pFoilWt,             SIGNAL(mouseDragReleased()),    SLOT(onTakePicture()));
    connect(m_pFoilWt,             SIGNAL(objectModified()),       SLOT(onTakePicture()));

    connect(m_prbBS,               SIGNAL(clicked(bool)),          SLOT(onSplineType()));
    connect(m_prbCS,               SIGNAL(clicked(bool)),          SLOT(onSplineType()));

    connect(m_pcbSplineDegree,     SIGNAL(activated(int)),         SLOT(onSplineDegree()));
    connect(m_pieNPanels,          SIGNAL(intChanged(int)),        SLOT(onOutputSize()));
    connect(m_plbSplineStyle,      SIGNAL(clickedLB(LineStyle)),   SLOT(onSplineStyle(LineStyle)));
    connect(m_pchShowCtrlPts,      SIGNAL(clicked(bool)),          SLOT(onShowCtrlPts(bool)));
    connect(m_pchShowNormals,      SIGNAL(clicked(bool)),          SLOT(onShowNormals(bool)));


    connect(m_pchClosedTE,         SIGNAL(clicked(bool)),          SLOT(onClosedTE()));
    connect(m_pslBunchAmp,         SIGNAL(sliderMoved(int)),       SLOT(onBunch(int)));

    QMenu *pActionsMenu = m_ppbActionsMenuBtn->menu();
    {
        QAction *pActReset      = new QAction("Reset spline",     this);
        QAction *pActApproxFoil = new QAction("Approximate foil", this);
        QAction *pActOverlay    = new QAction("Overlay foil",     this);

        connect(pActReset,      SIGNAL(triggered(bool)), SLOT(onMakeDefaultSpline()));
        connect(pActApproxFoil, SIGNAL(triggered(bool)), SLOT(onApproxFoil()));
        connect(pActOverlay,    SIGNAL(triggered(bool)), m_pFoilWt, SLOT(onOverlayFoil()));

        pActionsMenu->addAction(pActReset);
        pActionsMenu->addAction(pActApproxFoil);
        pActionsMenu->addAction(pActOverlay);
    }
}


void Foil1SplineDlg::showEvent(QShowEvent *pEvent)
{
    FoilDlg::showEvent(pEvent);
    resizeEvent(nullptr);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    m_pButtonBox->setFocus();
}


void Foil1SplineDlg::hideEvent(QHideEvent *pEvent)
{
    FoilDlg::hideEvent(pEvent);
    s_HSplitterSizes  = m_pHSplitter->saveState();
}


void Foil1SplineDlg::resizeEvent(QResizeEvent *)
{
    int h = m_pFoilWt->height();
    int w = m_pFoilWt->width();


    QPoint pos2(w-m_pButtonBox->width()-5, h-m_pButtonBox->height()-5);
    m_pButtonBox->move(pos2);
}


void Foil1SplineDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    switch (pEvent->key())
    {
        case Qt::Key_Y:
            if(bCtrl) onRedo();
            break;
        case Qt::Key_Z:
            if(bCtrl && bShift) onRedo();
            else if(bCtrl) onUndo();
            break;
        default:
            FoilDlg::keyPressEvent(pEvent);
            return;
    }
}


void Foil1SplineDlg::onSplitterMoved()
{
    resizeEvent(nullptr);
}


void Foil1SplineDlg::onUndo()
{
    if(s_bCubicSpline)
    {
        if(m_CStackPos>0)
        {
            m_CStackPos--;
            setPicture();
            m_ppbUndo->setEnabled(m_CStackPos>0);
            m_ppbRedo->setEnabled(m_CStackPos<m_CSplineStack.size()-1);
        }
        else
        {
            //nothing to restore
        }
    }
    else
    {
        if(m_BStackPos>0)
        {
            m_BStackPos--;
            setPicture();
            m_ppbUndo->setEnabled(m_BStackPos>0);
            m_ppbRedo->setEnabled(m_BStackPos<m_BSplineStack.size()-1);
        }
        else
        {
            //nothing to restore
        }
    }

}


void Foil1SplineDlg::onRedo()
{
    if(s_bCubicSpline)
    {
        if(m_CStackPos<m_CSplineStack.size()-1)
        {
            m_CStackPos++;
            setPicture();
            m_ppbUndo->setEnabled(m_CStackPos>0);
            m_ppbRedo->setEnabled(m_CStackPos<m_CSplineStack.size()-1);
        }
    }
    else
    {
        if(m_BStackPos<m_BSplineStack.size()-1)
        {
            m_BStackPos++;
            setPicture();
            m_ppbUndo->setEnabled(m_BStackPos>0);
            m_ppbRedo->setEnabled(m_BStackPos<m_BSplineStack.size()-1);
        }
    }
}


void Foil1SplineDlg::onApproxFoil()
{
    SplineApproxDlg dlg(this);
    int degree      = s_bCubicSpline ? 3 : s_BS.degree();
    int nCtrlPoints = s_bCubicSpline ? s_CS.ctrlPointCount() : s_BS.ctrlPointCount();
    dlg.initDialog(s_bCubicSpline, degree, nCtrlPoints);

    if(dlg.exec()!=QDialog::Accepted) return;

    QString foilname = dlg.selectedFoilName();

    if(!foilname.length()) return; // nothing to approximate

    Foil const *pRefFoil = Objects2d::foil(foilname);
    if(!pRefFoil) return;

    onTakePicture();

    if(s_bCubicSpline)
    {
        s_CS.setOutputSize(m_pieNPanels->value());
        pRefFoil->makeCubicSpline(s_CS, dlg.nCtrlPoints());
        m_pFoilWt->setSpline(&s_CS);
        fillCtrls(&s_CS);
    }
    else
    {
        pRefFoil->makeApproxBSpline(s_BS, dlg.degree(), dlg.nCtrlPoints(), m_pieNPanels->value());
        m_pFoilWt->setSpline(&s_BS);
        fillCtrls(&s_BS);
    }

    update();

    onTakePicture();
}


/**
  * Clears the stack starting at a given position.
  * @param the first stack element to remove
  */
void Foil1SplineDlg::clearBStack(int pos)
{
    for(int il=m_BSplineStack.size()-1; il>pos; il--)
    {
        m_BSplineStack.pop();
    }
    m_BStackPos = m_BSplineStack.size()-1;
}


/**
  * Clears the stack starting at a given position.
  * @param the first stack element to remove
  */
void Foil1SplineDlg::clearCStack(int pos)
{
    for(int il=m_CSplineStack.size()-1; il>pos; il--)
    {
        m_CSplineStack.pop();
    }
    m_CStackPos = m_CSplineStack.size()-1;
}


/**
 * Copies the current SplineFoil object to a new SplineFoil object and pushes it on the stack.
 */
void Foil1SplineDlg::onTakePicture()
{
    m_bModified = true;
    if(s_bCubicSpline)
    {
        clearCStack(m_CStackPos);
        m_CSplineStack.push(s_CS);
        m_CStackPos = m_CSplineStack.size()-1;

        m_ppbUndo->setEnabled(m_CStackPos>0);
        m_ppbRedo->setEnabled(m_CStackPos<m_CSplineStack.size()-1);
    }
    else
    {
        clearBStack(m_BStackPos);
        m_BSplineStack.push(s_BS);
        m_BStackPos = m_BSplineStack.size()-1;

        m_ppbUndo->setEnabled(m_BStackPos>0);
        m_ppbRedo->setEnabled(m_BStackPos<m_BSplineStack.size()-1);
    }

}


/**
 * Restores the Spline definition from the current position in the stack.
 */
void Foil1SplineDlg::setPicture()
{
    if(s_bCubicSpline)
    {
        if(m_CStackPos<0 || m_CStackPos>=m_CSplineStack.size()) return;
        s_CS.duplicate(m_CSplineStack.at(m_CStackPos));
        s_CS.updateSpline();
        s_CS.makeCurve();
    }
    else
    {
        if(m_BStackPos<0 || m_BStackPos>=m_BSplineStack.size()) return;
        s_BS.duplicate(m_BSplineStack.at(m_BStackPos));
        s_BS.updateSpline();
        s_BS.makeCurve();
    }
    update();
}


void Foil1SplineDlg::fillCtrls(Spline const*pSpline)
{
    if(!pSpline) return;

    m_pcbSplineDegree->setEnabled(pSpline->isBSpline());

    if(pSpline->isBSpline())
    {
        BSpline const*pBS = dynamic_cast<BSpline const*>(pSpline);
        if(pBS)
        {
            m_pcbSplineDegree->setCurrentIndex(pBS->degree()-2);
            m_pcbSplineDegree->setEnabled(true);
        }
    }
    else
    {
        m_pcbSplineDegree->setCurrentIndex(1);
        m_pcbSplineDegree->setEnabled(false);
    }

    m_pslBunchAmp->setValue(int(pSpline->bunchAmplitude()*100.0));

    m_pchClosedTE->setChecked(pSpline->isClosed());
    m_plbSplineStyle->setTheStyle(pSpline->theStyle());
    m_pieNPanels->setValue(pSpline->outputSize());
    m_pchShowCtrlPts->setChecked(pSpline->isVisible());
    m_pchShowNormals->setChecked(pSpline->bShowNormals());
}


void Foil1SplineDlg::onClosedTE()
{
    if(s_bCubicSpline) s_CS.setClosed(m_pchClosedTE->isChecked());
    else s_BS.setClosed(m_pchClosedTE->isChecked());

    m_pFoilWt->update();
}


void Foil1SplineDlg::onSplineStyle(LineStyle)
{
    Spline *pSpline(nullptr);
    if(s_bCubicSpline) pSpline = &s_CS;
    else               pSpline = &s_BS;

    LineMenu *m_pLineMenu = new LineMenu(nullptr, true);
    m_pLineMenu->initMenu(pSpline->stipple(), pSpline->width(), pSpline->color(), pSpline->pointStyle());
    m_pLineMenu->exec(QCursor::pos());
    LineStyle ls = m_pLineMenu->theStyle();
    m_plbSplineStyle->setTheStyle(ls);
    pSpline->setTheStyle(ls);

    m_pFoilWt->update();
}


void Foil1SplineDlg::onShowCtrlPts(bool bShow)
{
    Spline *pSpline(nullptr);
    if(s_bCubicSpline) pSpline = &s_CS;
    else               pSpline = &s_BS;
    pSpline->showCtrlPts(bShow);

    m_pFoilWt->update();
}


void Foil1SplineDlg::onShowNormals(bool bNormals)
{
    Spline *pSpline(nullptr);
    if(s_bCubicSpline) pSpline = &s_CS;
    else               pSpline = &s_BS;
    pSpline->showNormals(bNormals);

    m_pFoilWt->update();
}


void Foil1SplineDlg::onBunch(int)
{
    Spline *pSpline(nullptr);
    if(s_bCubicSpline) pSpline = &s_CS;
    else               pSpline = &s_BS;

    int val0 = m_pslBunchAmp->value();
    double amp = double(val0)/100.0;
    pSpline->setBunchParameters(Spline::DOUBLESIG, amp);

    pSpline->makeCurve();
    m_pFoilWt->update();
}


void Foil1SplineDlg::onOutputSize()
{
    Spline *pSpline(nullptr);
    if(s_bCubicSpline) pSpline = &s_CS;
    else               pSpline = &s_BS;
    pSpline->setOutputSize(m_pieNPanels->value());
    pSpline->makeCurve();
    m_pFoilWt->update();
}


void Foil1SplineDlg::onSplineDegree()
{
    if(s_bCubicSpline) return;
    int ideg = m_pcbSplineDegree->currentIndex()+2;
    if(s_BS.ctrlPointCount())
    {
        if(ideg<s_BS.ctrlPointCount())
        {
            // there are enough control points for this degree
            s_BS.setDegree(ideg);
        }
        else
        {
            // too few control points, adjust the degree
            QMessageBox::warning(this,"Warning", "The spline degree must be less than the number of control points");
            m_pcbSplineDegree->setCurrentIndex(s_BS.degree()-2);

            s_BS.setDegree(std::max(2,int(s_BS.ctrlPointCount())-1));

        }
        s_BS.updateSpline();
        s_BS.makeCurve();
    }
    m_pFoilWt->update();
}



