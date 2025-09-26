/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once
#include <QObject>
#include <QTime>
#include <QVector>

#include <xflobjects/objects3d/analysis/spandistribs.h>
#include <xflobjects/objects3d/analysis/aeroforces.h>
#include <xflpanels/panels/panel3.h>
#include <xflobjects/analysis3d/task3d.h>


class Boat;
class BoatPolar;
class BoatOpp;

class Vector3d;

class BoatTask : public Task3d
{
    Q_OBJECT

    public:
        BoatTask();

        void setObjects(Boat *pBoat, BoatPolar *pBtPolar);
        bool initializeTask(QObject *pParent);


        void setAnalysisRange(const QVector<double> &opplist);

        bool initializeTriangleAnalysis();

        void loop() override;

        double ctrlParam() const {return m_CtrlParam;}

        void scaleResultsToSpeed(int qrhs);
        BoatOpp *computeBoat(int qrhs);
        BoatOpp *createBtOpp(const double *Cp, const double *Mu, const double *Sigma);

        void setAngles(QVector<Panel3> &panels, double phi);

        int allocateSailResultsArrays();

        Boat *boat() const {return m_pBoat;}
        BoatPolar *btPolar() const {return m_pBtPolar;}
        QVector<BoatOpp*> const &BtOppList() const {return m_BtOppList;}

    private:
        void makeVortonRow(int qrhs) override;
        void computeInducedForces(double alpha, double beta, double QInf);
        void computeInducedDrag(double alpha, double beta, double QInf, int qrhs,
                                QVector<Vector3d> &WingForce, QVector<SpanDistribs> &SpanDist) const;
    signals:
        void updateLiveBtOpp(BoatOpp *);

    private:
        Boat *m_pBoat;
        BoatPolar *m_pBtPolar;
        BoatOpp *m_pLiveBtOpp;             /**< to update the 3d display during the analysis */

        QVector<double> m_OppList;
        double m_CtrlParam;

        QVector<Vector3d> m_SailForceFF;   /**< The array of calculated resulting forces acting on the sails in body axis (N/q) */
        QVector<Vector3d> m_SailForceSum;   /**< The array of calculated resulting forces acting on the sails in body axis (N/q) */
        QVector<Vector3d> m_HullForce;   /**< The array of calculated resulting forces acting on the fuses in body axis (N/q) */
        QVector<SpanDistribs> m_SpanDist; /**< the array of span distributions of the sails */

        //only one set of aerocoefs is needed, since unlike planes, points are calculated one at a time and stored in a BoatOpp
        AeroForces m_AF;

        QVector<BoatOpp*> m_BtOppList;
};

