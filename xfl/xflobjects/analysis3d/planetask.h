/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <xflobjects/analysis3d/task3d.h>
#include <xflcore/t8opp.h>
#include <xflobjects/objects3d/analysis/aeroforces.h>
#include <xflobjects/objects3d/analysis/spandistribs.h>
#include <xflobjects/objects3d/analysis/stabderivatives.h>
#include <xflpanels/panels/vorton.h>

#include <complex>

class Plane;
class PlaneXfl;
class WingXfl;
class WPolar;
class PlaneOpp;
class Node;
class Surface;
class Panel3;
class Panel4;
class AngleControl;

class PlaneTask : public Task3d
{
    Q_OBJECT
    public:
        PlaneTask();
        ~PlaneTask();

        void setOppList(QVector<double> const &opplist);
        void setCtrlOppList(QVector<double> const &opplist);
        void setStabOppList(QVector<double> const &opplist);
        void setT8OppList(QVector<T8Opp> const &ranges);

        double ctrl() const {return m_Ctrl;}
        double aoa()  const {return m_Alpha;}

        void setObjects(Plane *pPlane, WPolar *pWPolar);
        QList<PlaneOpp*> const & planeOppList() const {return m_PlaneOppList;}

        Plane *plane()  const {return m_pPlane;}
        WPolar*wPolar() const {return m_pWPolar;}
        void loop() override;
        bool initializeTask();
        void clearPOppList();

        void getVelocityVector(Vector3d const &C, double coreradius, bool bMultiThread, Vector3d &velocity) const;

        static void setViscousLoopSettings(bool bInitVTwist, double relaxfactor, double alphaprec, int maxiters);
        static void setMaxViscIter(int n) {s_ViscMaxIter=n;}
        static void setMaxViscError(double err) {s_ViscAlphaPrecision=err;}
        static void setViscRelaxFactor(double r) {s_ViscRelax=r;}
        static int maxViscIter() {return s_ViscMaxIter;}
        static double maxViscError() {return s_ViscAlphaPrecision;}
        static double viscRelaxFactor() {return s_ViscRelax;}
        static bool bViscInitVTwist() {return s_bViscInitTwist;}
        static void setViscInitVTwist(bool bInit) {s_bViscInitTwist=bInit;}


    public slots:
        void run() override;

    private:
        bool T123458Loop();
        bool T6Loop();
        bool T7Loop();

        void allocatePlaneResultArrays();
        void outputStateMatrices(PlaneOpp const *pPOpp);
        bool checkWPolarData(const Plane *pPlane, WPolar *pWPolar);
        double computeBalanceSpeeds(double Alpha, double mass, bool &bWarning, const QString &prefix, QString &log) const;
        double computeGlideSpeed(double Alpha, double mass, QString &log);
        void computeControlDerivatives(double t7ctrl, double alphaeq, double u0, StabDerivatives &SD);
        void outputNDStabDerivatives(double u0, const StabDerivatives &SD);
        PlaneOpp *computePlane(double ctrl, double Alpha, double Beta, double phi, double QInf, double mass, const Vector3d &CoG, bool bInGeomAxes);
        void computeInviscidAero(const QVector<Panel3> &panel3, const double *Cp3Vtx, const WPolar *pWPolar, double Alpha, AeroForces &AF) const;
        void computeInducedForces(double alpha, double beta, double QInf);
        void computeInducedDrag(double alpha, double beta, double QInf);
        bool computeViscousDrag(WingXfl *pWing, double alpha, double beta, double QInf, const WPolar *pWPolar, Vector3d const &cog, int iStation0, SpanDistribs &SpanResFF, QString &logmsg) const;
        bool computeViscousDragOTF(WingXfl *pWing, double alpha, double beta, double QInf, const WPolar *pWPolar, Vector3d const &cog, const AngleControl &TEFlapAngles, SpanDistribs &SpanResFF, QString &logmsg);
        PlaneOpp *createPlaneOpp(double ctrl, double alpha, double beta, double phi, double QInf, double mass, const Vector3d &CoG, const double *Cp, const double *Gamma, const double *Sigma, bool bCpOnly=false) const;
        void addTwistedVelField(double Qinf, double alpha, QVector<Vector3d> &VField) const;

        void scaleResultsToSpeed(double vOld, double vNew);

        void setControlPositions(PlaneXfl const*pPlaneXfl, WPolar const*pWPolar, QVector<Panel4> &panel4, double deltactrl, int iAVLCtrl, QString &outstring);
        void setControlPositions(PlaneXfl const *pPlaneXfl, WPolar const *pWPolar, QVector<Panel3> &panel3, QVector<Node> const &refnodes, double deltactrl, int iAVLCtrl, QString &outstring);
        void makeControlBC(PlaneXfl const*pPlaneXfl, WPolar const*pWPolar, Vector3d *normals, double deltactrl, int iAVLCtrl, QString &outstring);

        void makeVortonRow(int qrhs) override;

        bool updateVirtualTwist(double QInf, double &error, QString &log);

        bool setLinearSolution();

        bool computeStability(PlaneOpp *pPOpp, bool bOutput);


    private:

        Plane *m_pPlane;
        WPolar *m_pWPolar;
        QList<PlaneOpp*> m_PlaneOppList;

        double m_Ctrl;             /**< the oppoint currently calculated */
        double m_Alpha;            /**< the aoa currently calculated */
        double m_Beta;             /**< the current sideslip */
        double m_Phi;              /**< the current bank angle*/
        double m_QInf;             /**< the current freestream velocity */

        QVector<double> m_AngleList, m_T6CtrlList, m_T7CtrlList;   /**< The list of operating points to analyze for each polar type*/
        QVector<T8Opp> m_T8Opps;

        QVector<SpanDistribs> m_SpanDistFF; /**< the array of span distributions of the wings */
        QVector<Vector3d> m_WingForce;   /**< The array of calculated resulting forces acting on the wings in wind axis (N/q) */

        Vector3d m_Force0;  /** The calculated equilibrium force  @todo check body or wind axis*/
        Vector3d m_Moment0; /** The calculated equilibrium moment @todo check body or wind axis */

        // temp variables used to create the operating point
        AeroForces m_AF;               /** the overall aero forces acting on the plane */
        QVector<AeroForces> m_PartAF;  /** the array of Aero forces acting on each part, for each operating point */


    private:
        static bool s_bViscInitTwist;
        static double s_ViscRelax;
        static double s_ViscAlphaPrecision;
        static int s_ViscMaxIter;

};

