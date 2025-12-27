
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

#include <QDebug>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QFileDialog>


#include "foil1splinedlg.h"
#include <interfaces/controls/splinectrl/splineapproxdlg.h>
#include <interfaces/editors/foiledit/foilwt.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/spline.h>
#include <api/constants.h>
#include <interfaces/widgets/customdlg/intvaluesdlg.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <api/utils.h>


QByteArray Foil1SplineDlg::s_HSplitterSizes;
Spline::enumType Foil1SplineDlg::s_SplineType(Spline::BSPLINE);
BSpline Foil1SplineDlg::s_BS;
BezierSpline Foil1SplineDlg::s_BZ;
CubicSpline Foil1SplineDlg::s_CS;


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
        s_BS.setColor(xfl::Turquoise);
    }

    if(s_BZ.ctrlPointCount()==0)
    {
        s_BZ.resetSpline();
        s_BZ.makeDefaultControlPoints(m_pchClosedTE->isChecked(), false);
        s_BZ.updateSpline();
        s_BZ.makeCurve();
        s_BZ.setColor(xfl::GreenYellow);
    }

    s_CS.setBunchType(Spline::DOUBLESIG);
    if(s_CS.ctrlPointCount()==0)
    {
        s_CS.resetSpline();
        s_CS.makeDefaultControlPoints(m_pchClosedTE->isChecked(), false);
        s_CS.updateSpline();
        s_CS.makeCurve();
        s_CS.setColor(xfl::PhugoidGreen);
    }
}


void Foil1SplineDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);

    m_pFoilWt->setBufferFoil(nullptr);
    m_pBufferFoil->hide();
    m_pBufferFoil->setName(std::string("Splined foil"));

    m_BSplineStack.clear();
    m_BSplineStack.push(s_BS);
    m_BStackPos = 0;

    m_CSplineStack.clear();
    m_CSplineStack.push(s_CS);
    m_CStackPos = 0;

    //clean up past mistakes
    if(s_BS.outputSize()<=0) s_BS.setOutputSize(79);
    if(s_BZ.outputSize()<=0) s_BZ.setOutputSize(79);
    if(s_CS.outputSize()<=0) s_CS.setOutputSize(79);

    if(isBSpline())
    {
         m_pFoilWt->setSpline(&s_BS);
         fillCtrls(&s_BS);
    }
    else if(isBezier())
    {
         m_pFoilWt->setSpline(&s_BZ);
         fillCtrls(&s_BZ);
    }
    else if(isCubic())
    {
         m_pFoilWt->setSpline(&s_CS);
         fillCtrls(&s_CS);
    }
    s_BS.setVisible(isBSpline());
    s_BZ.setVisible(isBezier());
    s_CS.setVisible(isCubic());

    m_prbBS->setChecked(isBSpline());
    m_prbBZ->setChecked(isBezier());
    m_prbCS->setChecked(isCubic());

    m_pActApproxFoil->setEnabled(!isBezier());

    if(!m_pRefFoil) onApply();
}


Spline *Foil1SplineDlg::getActiveSpline()
{
    if      (isBSpline()) return &s_BS;
    else if (isBezier())  return &s_BZ;
    else if (isCubic())   return &s_CS;

    return nullptr; // bad
}


void Foil1SplineDlg::onApply()
{
    const Spline *pSpline = getActiveSpline();

    std::vector<Node2d> basenodes;

    if(!pSpline->issymmetric())
    {
        int npts = pSpline->outputSize();
        basenodes.resize(npts);
        for(int i=0; i<npts; i++)
        {
            basenodes[i] = pSpline->outputPt(i);
        }
    }
    else
    {
        int npts = pSpline->outputSize();
        basenodes.resize(npts*2-1);
        for(int i=0; i<npts; i++)
        {
            basenodes[i] = pSpline->outputPt(i);
        }

        //make symmetric half
        int j = npts;
        for(int i=npts-2; i>=0; i--)
        {
            basenodes[j] = pSpline->outputPt(i);
            j++;
        }
    }

    m_pBufferFoil->setBaseNodes(basenodes);
    m_pBufferFoil->initGeometry();

    m_pFoilWt->update();

}


void Foil1SplineDlg::onSplineType()
{
    if      (m_prbBS->isChecked()) s_SplineType = Spline::BSPLINE;
    else if (m_prbBZ->isChecked()) s_SplineType = Spline::BEZIER;
    else if (m_prbCS->isChecked()) s_SplineType = Spline::CUBIC;

    s_BS.setVisible(s_SplineType==Spline::BSPLINE);
    s_BZ.setVisible(s_SplineType==Spline::BEZIER);
    s_CS.setVisible(s_SplineType==Spline::CUBIC);

    if(s_SplineType==Spline::BSPLINE)
    {
        m_pFoilWt->setSpline(&s_BS);
        fillCtrls(&s_BS);
    }
    else if(s_SplineType==Spline::BEZIER)
    {
        m_pFoilWt->setSpline(&s_BZ);
        fillCtrls(&s_BZ);
    }
    else if(s_SplineType==Spline::CUBIC)
    {
        m_pFoilWt->setSpline(&s_CS);
        fillCtrls(&s_CS);
    }

    m_pActApproxFoil->setEnabled(!isBezier());

    m_pFoilWt->update();
}


void Foil1SplineDlg::onMakeDefaultSpline()
{
    Spline *pSpline(nullptr);
    if(isCubic())
    {
        pSpline = &s_CS;
        clearCStack(0);
    }
    else if(isBSpline())
    {
        pSpline = &s_BS;

        int ideg = m_pcbSplineDegree->currentIndex()+2;
        pSpline->setDegree(ideg);

        clearBStack(0);
    }

    s_BS.setVisible(isBSpline());
    s_BZ.setVisible(isBezier());
    s_CS.setVisible(isCubic());

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
                        m_prbBZ = new QRadioButton("Bezier");
                        m_prbCS = new QRadioButton("Cubic");

                        m_prbBZ->setVisible(false);
                        m_prbBZ->setEnabled(false);

                        pSplineLayout->addStretch();
                        pSplineLayout->addWidget(m_prbBS);
                        pSplineLayout->addWidget(m_prbBZ);
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

                        m_pchClosedTE = new QCheckBox("Force closed T.E.");
                        m_pchClosedTE->setToolTip("<p>Forces the spline's leading and trailing points to be at the same position</p>");


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
    connect(m_prbBZ,               SIGNAL(clicked(bool)),          SLOT(onSplineType()));
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
        m_pActApproxFoil        = new QAction("Approximate foil", this);
        QAction *pActOverlay    = new QAction("Overlay foil",     this);

        connect(pActReset,        SIGNAL(triggered(bool)), SLOT(onMakeDefaultSpline()));
        connect(m_pActApproxFoil, SIGNAL(triggered(bool)), SLOT(onApproxFoil()));
        connect(pActOverlay,      SIGNAL(triggered(bool)), m_pFoilWt, SLOT(onOverlayFoil()));

        pActionsMenu->addAction(pActReset);
        pActionsMenu->addAction(m_pActApproxFoil);
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
    if(isCubic())
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
    else if(isBSpline())
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
    if(isCubic())
    {
        if(m_CStackPos<m_CSplineStack.size()-1)
        {
            m_CStackPos++;
            setPicture();
            m_ppbUndo->setEnabled(m_CStackPos>0);
            m_ppbRedo->setEnabled(m_CStackPos<m_CSplineStack.size()-1);
        }
    }
    else if(isBSpline())
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
    if(isBezier()) return;

    SplineApproxDlg dlg(this);
    int degree = 3;
    if( isBSpline()) degree = s_BS.degree();

    int nCtrlPoints = 0;

    if(isCubic()) nCtrlPoints = s_CS.nCtrlPoints();
    if(isBSpline()) nCtrlPoints = s_BS.nCtrlPoints();

    dlg.initDialog(isCubic(), degree, nCtrlPoints);

    if(dlg.exec()!=QDialog::Accepted) return;

    QString foilname = dlg.selectedFoilName();

    if(!foilname.length()) return; // nothing to approximate

    Foil const *pRefFoil = Objects2d::foil(foilname.toStdString());
    if(!pRefFoil) return;

    onTakePicture();

    if(isCubic())
    {
        s_CS.setOutputSize(m_pieNPanels->value());
        pRefFoil->makeCubicSpline(s_CS, dlg.nCtrlPoints());
        m_pFoilWt->setSpline(&s_CS);
        fillCtrls(&s_CS);
    }
    else if(isBSpline())
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
    if(isCubic())
    {
        clearCStack(m_CStackPos);
        m_CSplineStack.push(s_CS);
        m_CStackPos = m_CSplineStack.size()-1;

        m_ppbUndo->setEnabled(m_CStackPos>0);
        m_ppbRedo->setEnabled(m_CStackPos<m_CSplineStack.size()-1);
    }
    else if(isBSpline())
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
    if(isCubic())
    {
        if(m_CStackPos<0 || m_CStackPos>=m_CSplineStack.size()) return;
        s_CS.duplicate(m_CSplineStack.at(m_CStackPos));
        s_CS.updateSpline();
        s_CS.makeCurve();
    }
    else if(isBSpline())
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
    if     (isBSpline()) s_BS.setClosed(m_pchClosedTE->isChecked());
    else if(isBezier())  s_BZ.setClosed(m_pchClosedTE->isChecked());
    else if(isCubic())   s_CS.setClosed(m_pchClosedTE->isChecked());

    m_pFoilWt->update();
}


void Foil1SplineDlg::onSplineStyle(LineStyle)
{
    Spline *pSpline = getActiveSpline();

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
    Spline *pSpline = getActiveSpline();
    pSpline->showCtrlPts(bShow);

    m_pFoilWt->update();
}


void Foil1SplineDlg::onShowNormals(bool bNormals)
{
    Spline *pSpline = getActiveSpline();

    pSpline->showNormals(bNormals);

    m_pFoilWt->update();
}


void Foil1SplineDlg::onBunch(int)
{
    Spline *pSpline = getActiveSpline();

    int val0 = m_pslBunchAmp->value();
    double amp = double(val0)/100.0;
    pSpline->setBunchParameters(Spline::DOUBLESIG, amp);

    pSpline->makeCurve();
    m_pFoilWt->update();
}


void Foil1SplineDlg::onOutputSize()
{
    Spline *pSpline = getActiveSpline();
    pSpline->setOutputSize(m_pieNPanels->value());
    pSpline->makeCurve();
    m_pFoilWt->update();
}


void Foil1SplineDlg::onSplineDegree()
{
    if(isCubic()) return;
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



