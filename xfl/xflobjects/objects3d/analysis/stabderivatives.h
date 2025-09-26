/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QVector>

class StabDerivatives
{
    public:
        StabDerivatives();

        void setMetaData(double span, double mac, double area, double u0, double mass, double CoG_x, double rho);

        void reset();

        void resizeControlDerivatives(int n);

        void computeNDStabDerivatives();
        bool serializeFl5(QDataStream &ar, bool bIsStoring);

        //--------------- VARIABLES --------------

        // Operating point data
        double m_Span, m_Area, m_MAC;
        double m_QInf;
        double m_Mass;
        double m_CoG_x;
        double m_rho;


        // DIMENSIONAL derivatives
        // longitudinal
        double Xu, Xw, Zu, Zw, Xq, Zq, Mu, Mw, Mq;//first order
        double Zwp, Mwp;                          //second order derivatives, cannot be calculated by a panel method, set to zero.
        // lateral
        double Yv, Yp, Yr, Lv, Lp, Lr, Nv, Np, Nr;//first order

        // control derivatives

        QVector<QString> ControlNames;
        QVector<double> Xde, Yde, Zde, Lde, Mde, Nde;

        // NON DIMENSIONAL derivatives
        // stability
        double CXu, CZu, Cmu, CXa, CZa, Cma, CXq, CZq, Cmq, CYb, CYp, CYr, Clb, Clp, Clr, Cnb, Cnp, Cnr;
        // control
        QVector<double>  CXe,CYe,CZe;
        QVector<double>  CLe,CMe,CNe;

        // result
        double XNP;                 /**< Neutral point x-position resulting from stability analysis */

};
