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

#include <vector>

/**
*@brief
*	This class implements the operating point object which stores the data of plane analysis
*
    The WingOpp is always a member variable of a PlaneOpp object.
    The data is stored in International Standard Units, i.e. meters, seconds, kg, and Newtons.
    Angular data is stored in degrees.
*/


#include <complex>

#include <vector3d.h>
#include <aeroforces.h>
#include <spandistribs.h>
#include <wingxfl.h>


class WingXfl;
class PlanePolar;

class FL5LIB_EXPORT WingOpp
{
    public:
        WingOpp(int PanelArraySize=0);

    public:
        //________________METHODS____________________________________

        bool serializeWingOppXFL(QDataStream &ar, bool bIsStoring);
        bool serializeWingOppFl5(QDataStream &ar, bool bIsStoring);

        double maxLift() const;
        void createWOpp(const WingXfl *pWing, const PlanePolar *pWPolar, const SpanDistribs &distribs, const AeroForces &AF);

        std::string const &wingName() const {return m_WingName;}

        void setAeroForces(AeroForces const &af) {m_AF=af;}
        AeroForces const & aeroForces() const {return m_AF;}

        void setSpanResults(SpanDistribs const &distribs);
        SpanDistribs const &spanResults() const {return m_SpanDistrib;}

    private:
        std::string m_WingName;	// the wing name to which the WingOpp belongs


    public:
        bool m_bOut;         /**< true if there was an interpolation error of the viscous properties for this WingOpp */

        int m_nPanel4;       /**< the number of panels */
        int m_NStation;      /**< the number of stations along the span */
        int m_nFlaps;        /**< the number of trailing edge flaps */

        xfl::enumType m_WingType;

        double *m_dCp;                           /**< a pointer to the array of pressure coefficient for each panel */
        double *m_dG;                            /**< a pointer to the array of vortice or doublet strengths */
        double *m_dSigma;                        /**< a pointer to the array of source strengths */

        double m_Span;                          /**< the parent's Wing span */
        double m_MAChord;                       /**< the parent's Wing mean aerodynamic chord*/

        double m_MaxBending;  /**< the bending moment at the root chord */

        std::vector<double> m_FlapMoment;   /**< the flap hinge moments */

        AeroForces m_AF;

        SpanDistribs m_SpanDistrib;
};



