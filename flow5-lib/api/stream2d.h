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

#include <foil.h>
#include <panel2d.h>
#include <vector2d.h>
#include <constants.h>


/**
 * Implements Mark Drela's 2d panel analysis as described in
 * "XFoil: An analysis and design system for low Reynolds number Airfoils"
 * Low Reynolds Number Aerodynamics. Springer-Verlag Lec. Notes in Eng. 54. 1989.
 *
 *
 * Note: the main difficulty of this method is the discontinuity of the stream
 * function of a source panel across the line supporting the panel.
 * This causes instabilities in the linear solution.
 * XFoil somehow eliminated this problem.
 */

// the offset in a panel's normal direction, used when calculating a quantity on the 2d panel
#define OFFSET 0.00001


class FL5LIB_EXPORT Stream2d
{    
    static void setDefaultValues();

    public:
        Stream2d();
        ~Stream2d();


        void setFoil(const Foil *pFoil);
        void setTEDensities();

        void resetViscousSolution();
        bool solve();
        bool calcSolution(double alpha, double Q);
        void makeAij();
        void makeBij(std::vector<double> &bij, double sign);
        void makeBpij();
        void makeRHS(std::vector<double> &rhs, double alpha, double Q);

        void resizeSourceArrays();

        void getVelocity(double alpha, double qinf, const Vector2d &pt, Vector2d &vel, bool bSigma=true) const;

        double streamValue(double alpha, double qinf, Vector2d const &pt) const;

        int getLEindex(double alpha, double qinf) const;

        double Cpv(double alpha, double qinf, int iNode) const;

        void allocateAnalysisArrays(int nPts);

        void checkSolution(double alpha, double qinf) const;
        void listCirculations() const;

        double getZeroLiftAngle();

        double surfaceVelocity(double alpha, double qinf, int iNode) const;
        double gamma(int iNode) const;

        double psi() const {return m_Psi;}
        Foil const *pFoil() const {return &m_Foil;}

        int matSize() const {return m_matsize;}

        double Cl()  const {return m_CL;}
        double Cm()  const {return m_Cm;}
        double XCP() const {return m_XCP;}

        std::vector<Node2d> const&nodes() const {return m_Node;}
        std::vector<Node2d> &nodes() {return m_Node;}
        Node2d const &node2d(int index) const {return m_Node.at(index);}
        int nNodes() const {return int(m_Node.size());}
        int nAirfoilNodes() const;

        int nPanels() const {return int(m_Panel.size());}
        int nAirfoilPanels() const;
        int nWakePanels() const;
        int firstWakePanelIndex() const;

        void getCpCurve(double alpha, double qinf, std::vector<double> &x, std::vector<double> &Cp) const;

        double alpha0() const {return m_Alpha0;}

        std::vector<double> const &dstar() const {return m_dstar;}

        void makeFoilPanels(Foil *pFoil);
        void makeWakePanels(double alpha, double qinf, float xmax=LARGEVALUE);
        void clearWakePanels();

        Panel2d const &panel2d(int index) const {return m_Panel.at(index);}
        Panel2d &panel2d(int index) {return m_Panel[index];}

        std::vector<Panel2d> const &panels() const {return m_Panel;}

        void calcOpPoint(double alpha, double qinf);

        void checkProfile(double alpha, double qinf, int iNode) const;

        void makeUe(double alpha, double qinf, const std::vector<Node2d> &n2d, std::vector<double> &Ue);
        Foil*makeBlasiusSigma(double alpha, double qinf, double coef=1.0, bool bMakeFoil=false);

        void makeSigma(const std::vector<float> &dstar, float ue, int iLE);

        void makeCpArray(double alpha, double qinf, std::vector<double> &x, std::vector<double>&Cp) const;

        bool hasLinearSolution() const {return m_bLinearSolution;}

        void setAirfoilSourceStrengths(float src); // debug only
        void setWakeSourceStrengths(float src);    // debug only

        static void setDefaults();

        static double wakeLength() {return s_WakeLength;}
        static void setWakeLength(double l) {s_WakeLength=l;}

        static double wakeProgressionFactor() {return s_WakeProgressionFactor;}
        static void setWakeProgressionFactor(double f) {s_WakeProgressionFactor=f;}

        static double firstWakePanelLength() {return s_FirstWakePanelLength;}
        static void setFirstWakePanelLength(double l) {s_FirstWakePanelLength=l;}

        static bool adjustFirstWakePanel() {return s_bAdjustFirstWakePanel;}
        static void setAdjustFirstWakePanel(bool b) {s_bAdjustFirstWakePanel=b;}


    protected:

        Foil m_Foil;
        int m_matsize;

        bool m_bLinearSolution; /**< true if the influence matrix has been sucessfully inverted and the unit results for 0° and 90 °have been calculated */

        double m_Alpha0;

        double m_CL;   /** the inviscid lift coefficient */
        double m_Cm;   /** the inviscid moment coefficient at the quarter chord point */
        double m_XCP;  /** the inviscid center of pressure position */

        std::vector<Node2d> m_Node;    /** the array of nodes, consisting of all foil nodes, and all wake nodes; the TE is not a node, i.e. the first wake node is the downstream point of the first wake panel */
        std::vector<Panel2d> m_Panel;  /** the array of panels, consisting of all foil nodes, and all wake panels; the TE panel in case of a blunt TE is not part of this array */

        std::vector<double> m_RHS;           /** the rhs for one aoa */

        std::vector<double> m_srcBL;           /** the array of source strengths = gradient of mass defects*/
        std::vector<std::vector<double>> m_dqtdsig;   /** the matrix of source influence coefficients d(QTan_i)/d(Sigma_j)*/
        std::vector<double> m_dstar; /** debug use only */

        Panel2d m_TEPanel;        /** The panel which closes the blunt TE; cf. Drela 1989 eq. (2)*/

        static double s_WakeLength;
        static double s_WakeProgressionFactor;
        static bool s_bAdjustFirstWakePanel;
        static double s_FirstWakePanelLength;

    private:
        std::vector<double> m_aij;    /** the (N+1)x(N+1) matrix of the linear vortex influence coefficients Drela eq.7*/
        std::vector<double> m_bpij;   /**  = aij-1 * bij = the matrix of mass defect influence on airfoil vorticities; Drela eq. 10 */

        std::vector<double> m_gamma_inv;     /** the array of vortex circulations resulting from the inviscid calculation */
        std::vector<double> m_gamma_src;     /** the array of vortex circulations resulting from the source distribution */

        std::vector<double> m_gam0, m_gam90; /** the vortex circulations for alpha=0 and 90° */

        double m_sigTE;           /** the source strength across the trailing edge panel */
        double m_gamTE;           /** the vortex strength across the trailing edge panel */

        double m_Psi;            // debug only the resulting value of the stream function on the airfoil's contour


};

