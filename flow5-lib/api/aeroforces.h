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


#include <QDataStream>

#include <fl5lib_global.h>


#include <cartesianframe.h>
#include <vector3d.h>
#include <panelprecision.h>


/**
 * @brief The AeroForces struct
 * This structure holds the aerodynamic forces acting on the plane for a given
 * operating point defined by the angle of attack, slip angle and velocity
 */
struct FL5LIB_EXPORT  AeroForces
{
    AeroForces();

    void duplicate(AeroForces const & ac);
    void resetAll();
    void resetResults();
    void scaleForces(double q);

    double alpha() const {return m_Alpha;}
    double beta()  const {return m_Beta;}
    double qInf()  const {return m_QInf;}

    void setOpp(double alfa, double beta, double phi, double vinf) {m_Alpha=alfa; m_Beta=beta; m_Phi=phi, m_QInf=vinf; makeFrames();}

/*        void setAlpha(double al) {m_Alpha=al; makeFrame();}
    void setBeta( double b)  {m_Beta=b;   makeFrame();}
    void setQInf(double v) {m_QInf=v;} */

    void makeFrames();

    void setReferenceArea( double refarea)  {m_RefArea=refarea;}
    void setReferenceChord(double refchord) {m_RefChord=refchord;}
    void setReferenceSpan( double refspan)  {m_RefSpan=refspan;}
    void setReferenceDims( double refarea, double refchord, double refspan) {m_RefArea=refarea;   m_RefChord=refchord;   m_RefSpan=refspan;}

    double refArea()  const {return m_RefArea;}
    double refChord() const {return m_RefChord;}
    double refSpan()  const {return m_RefSpan;}

    // Coefficients in Body Axes
    double Cx() const {if(m_RefArea>MINREFAREA) return m_Fff.x/m_RefArea; else return 0.0;} // in Body axes
    double Cy() const {if(m_RefArea>MINREFAREA) return m_Fff.y/m_RefArea; else return 0.0;} // in Body axes
    double Cz() const {if(m_RefArea>MINREFAREA) return m_Fff.z/m_RefArea; else return 0.0;} // in Body axes

    double Cx_sum() const {if(m_RefArea>MINREFAREA) return m_Fsum.x/m_RefArea; else return 0.0;} // in Body axes
    double Cy_sum() const {if(m_RefArea>MINREFAREA) return m_Fsum.y/m_RefArea; else return 0.0;} // in Body axes
    double Cz_sum() const {if(m_RefArea>MINREFAREA) return m_Fsum.z/m_RefArea; else return 0.0;} // in Body axes

    // Coefficients in Wind Axes
    double CL()    const;
    double CSide() const;
    double CDi()   const;
    double CDv()   const {return m_RefArea>MINREFAREA ? (m_ProfileDrag+m_FuseDrag+m_ExtraDrag)/m_RefArea    : 0.0;}
    double CD()    const {return CDi()+CDv();}

    // The roll such that the starboard wing goes down is >0
    // Opposite to algebraic result in body axis
    double Cli() const;

    // The pitching moment nose up is >0
    // Same as algebraic result in body axis
    double Cmi() const;
    double Cmv() const;
    double Cm()  const;

    // The yaw such that the nose goes to starboard	is >0
    // Opposite to algebraic result in body axis
    double Cni() const;
    double Cnv() const;
    double Cn()  const;

    Vector3d centreOfPressure() const;
    void setM0(Vector3d M0) {m_M0=M0;}
    Vector3d const &M0() const {return m_M0;}              // N.m/q

    Vector3d const &Fff() const {return m_Fff;}            // N/q
    void setFff(Vector3d const &Fff) {m_Fff=Fff;}          // N/q
    void addFff(Vector3d const &Fff) {m_Fff+=Fff;}         // N/q

    Vector3d const &Fsum() const {return m_Fsum;}          // N/q
    void setFsum(Vector3d const &Fsum) {m_Fsum=Fsum;}      // N/q
    void addFsum(Vector3d const &Fsum) {m_Fsum+=Fsum;}     // N/q

    Vector3d const &Mi() const {return m_Mi;}              // N.m/q
    void setMi(Vector3d const &Mi) {m_Mi=Mi;}              // N.m/q
    void addMi(Vector3d const &Mi) {m_Mi+=Mi;}             // N.m/q


    Vector3d const &Mv() const {return m_Mv;}              // N.m/q
    void setMv(Vector3d const &Mv) {m_Mv=Mv;}              // N.m/q
    void addMv(Vector3d const &Mv) {m_Mv+=Mv;}             // N.m/q

    double fffx() const {return m_Fff.x;}
    double fffy() const {return m_Fff.y;}
    double fffz() const {return m_Fff.z;}

    double fsumx() const {return m_Fsum.x;}
    double fsumy() const {return m_Fsum.y;}
    double fsumz() const {return m_Fsum.z;}

    double viscousDrag() const {return m_ProfileDrag + m_FuseDrag + m_ExtraDrag;}  //N/q

    double profileDrag() const {return m_ProfileDrag;}  //N/q
    void setProfileDrag(double d) {m_ProfileDrag=d;}
    void addProfileDrag(double d) {m_ProfileDrag+=d;}

    double fuseDrag() const {return m_FuseDrag;}  //N/q
    void setFuseDrag(double d) {m_FuseDrag=d;}
    void addFuseDrag(double d) {m_FuseDrag+=d;}

    double extraDrag() const {return m_ExtraDrag;}  //N/q
    void setExtraDrag(double d) {m_ExtraDrag=d;}
    void addExtraDrag(double d) {m_ExtraDrag+=d;}

    void serializeFl5_b17(QDataStream &ar, bool bIsStoring);
    bool serializeFl5(QDataStream &ar, bool bIsStoring);

    void displayAF();

    CartesianFrame const & CFWind() const {return m_CFWind;}
    CartesianFrame const & CFStab() const {return m_CFStab;}

    private:

        double m_Alpha;
        double m_Beta;
        double m_Phi;
        double m_QInf;

        double m_RefArea;
        double m_RefSpan;
        double m_RefChord;

        double m_ProfileDrag;         /**< The viscous profile drag in wind axis (N/q) = VCD.refArea - Only non-zero component is Fv.x */
        double m_FuseDrag;            /**< The viscous fuse drag in wind axis (N/q) = Cf.wettedArea m_AVLDrag- Only non-zero component is Fv.x */
        double m_ExtraDrag;           /**< The viscous extra drag + AVL type drag in wind axis (N/q) = Sum(coefs*area) - Only non-zero component is Fv.x */

        Vector3d m_M0;                /**< = Sum(Fi_j.r_j) =used to calculated the position of the center of pressure  (N.m/q) */

        Vector3d m_Fff;               /**< The resultant vector of induced/pressure forces in body axes in the far field plane (N/q) */
        Vector3d m_Fsum;              /**< The resultant vector of induced/pressure forces in body axes calculated by summation of panel forces (N/q) */

        Vector3d m_Mi;                /**< The moment of pressure forces w.r.t. polar COG in body axes (N.m/q) */
        Vector3d m_Mv;                /**< The moment of viscous  forces w.r.t. polar COG in body axes (N.m/q) */

        CartesianFrame m_CFWind;      /**< the Cartesian frame defining the wind axes;
                                             x aft aligned with wind, y unchanged, z perpendicular to x and pointing up;
                                            Note: this is not the true wind frame which would be rotated by the sideslip angle;
                                            it is meant to be identical to AVL's "inverted stability axes"*/
        CartesianFrame m_CFStab;      /**< the Cartesian frame defining the stability axes; x forward aligned with wind, y unchanged, z perpendicular to x and pointing down */

};


