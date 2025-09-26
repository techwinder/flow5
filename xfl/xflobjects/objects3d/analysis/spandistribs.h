/****************************************************************************
 *
 * 	flow5 application
 *
 * 	Copyright (C) Andre Deperrois 
 *
 * 	All rights reserved.
 *
 *****************************************************************************/


#pragma once

#include <QVector>

#include <xflgeom/geom3d/vector3d.h>

class WingXfl;

struct SpanDistribs
{
    public:
        SpanDistribs();
        void resizeResults(int NStation);
        void resizeGeometry(int NStation);
        void setGeometry(WingXfl const*pWing);

        bool serializeSpanResultsFl5(QDataStream &ar, bool bIsStoring);

        void clearGeometry();

        double stripLift(int m, double qDyn) const;
        double stripArea(int m) const {return m_StripArea.at(m);}

        void initializeToZero();

        int nStations() const {return m_Cl.size();}

    public:
        QVector<double> m_Ai;            /**< the induced angles, in degrees */
        QVector<double> m_Alpha_0;       /**< the zero-lift angle at the span station */
        QVector<double> m_Cl;            /**< the lift coefficient on the strips */
        QVector<double> m_ICd;           /**< the induced drag coefficient on the strips */
        QVector<double> m_PCd;           /**< the viscous drag coefficient on the strips */
        QVector<double> m_Re;            /**< the Reynolds number on the strips */
        QVector<double> m_XTrTop;        /**< the upper transition location on the strips */
        QVector<double> m_XTrBot;        /**< the lower transition location on the strips */
        QVector<double> m_CmViscous;     /**< the pitching moment of viscous drag on the strips, w.r.t CoG, normalized by the strip's chord and area  */
        QVector<double> m_CmPressure;    /**< the pitching moment of the pressure forces on the strips, w.r.t CoG, normalized by the strip's chord and area  */
        QVector<double> m_CmC4;          /**< the pitching moment coefficient on the strips w.r.t. the chord's quarter point, normalized by the strip's chord and area */
        QVector<double> m_XCPSpanRel;    /**< the relative position of the strip's center of pressure on the strips as a % of the local chord length*/
        QVector<double> m_XCPSpanAbs;    /**< the absolute position of the strip's center of pressure pos on the strips */
        QVector<double> m_BendingMoment; /**< the bending moment on the strips */
        QVector<double> m_VTwist;        /**< the virtual twist in viscous loops */
        QVector<double> m_Gamma;         /**< the circulation on the strip */
        QVector<bool> m_bOut;            /**< true if the local viscous interpolation has failed */
        QVector<Vector3d> m_Vd;          /**< the downwash vector at span stations in m/s. The downwash is calculated at the mid wake point, i.e. where the induced drag is evaluated. */
        QVector<Vector3d> m_F;           /**< the force vector at span stations, in N and in body axes */


        QVector<double> m_Chord;         /**< the chord on the strips */
        QVector<double> m_Offset;        /**< the offset at the span stations */
        QVector<double> m_Twist;         /**< the twist at the span stations */
        QVector<double> m_StripArea;     /**< the area of each chordwise strip */
        QVector<double> m_StripPos;       /**< the span positions of the stations */
        QVector<Vector3d> m_PtC4;        /**< the quarter chord points */
};

