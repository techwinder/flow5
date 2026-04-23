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

#include <QString>

#include <algorithm>
#include <thread>

#include <stream2d.h>

#include <constants.h>
#include <foil.h>
#include <matrix.h>
#include <panel2d.h>
#include <sgsmooth.h>
#include <vector2d.h>
#include <utils.h>


double Stream2d::s_WakeLength = 1.5;
double Stream2d::s_WakeProgressionFactor = 1.1;
bool Stream2d::s_bAdjustFirstWakePanel = true;
double Stream2d::s_FirstWakePanelLength = 0.01;


Stream2d::Stream2d()
{
    m_Alpha0 = 0.0;
    m_matsize = 0;

    m_CL = m_Cm = m_XCP = 0.0;

    m_bLinearSolution = false;

    m_sigTE = m_gamTE = 0.0;
    m_Psi = 0.0;
}


Stream2d::~Stream2d()
{
}


void Stream2d::setDefaults()
{
    s_WakeLength = 1.5;
    s_WakeProgressionFactor = 1.1;
    s_bAdjustFirstWakePanel = true;
    s_FirstWakePanelLength = 0.01;
}



/** Foil panels are built in the same anti-clockwise direction as the nodes
 * Panel nNodes links node nNodes and nNodes+1
 * The tangential vector is in the direction N_n+1, N_n so that the normal points outwards
 */
void Stream2d::makeFoilPanels(Foil *pFoil)
{
    if (!pFoil) return;

    m_Node.clear();
    m_Panel.clear();
    m_Panel.resize(pFoil->nNodes()-1);
    for(int p=0; p<pFoil->nNodes()-1; p++)
    {
        Node2d A(pFoil->x(p),   pFoil->y(p));
        Node2d B(pFoil->x(p+1), pFoil->y(p+1));
        A.setNormal(pFoil->normal(p));
        B.setNormal(pFoil->normal(p+1));
        m_Node.push_back(A);
        m_Node.back().setIndex(p);

        m_Panel[p].setFrame(A, B);
        m_Panel[p].A.setIndex(p);
        m_Panel[p].B.setIndex(p+1);
        m_Panel[p].setIndex(p);
    }
    Node2d A(pFoil->backNode());
    A.setNormal(pFoil->normal(pFoil->nNodes()-1));
    m_Node.push_back(A);
    m_Node.back().setIndex(pFoil->nNodes()-1);

    if(!pFoil->sharpTE())
    {
        // add the TE panel
        int npts = pFoil->nNodes();
        m_TEPanel.setFrame(m_Foil.node(pFoil->nNodes()-1),
                           m_Foil.node(0),
                           npts-1, true);
        m_TEPanel.A.setIndex(npts-1);
        m_TEPanel.B.setIndex(0);
    }
}

void Stream2d::resetViscousSolution()
{
    m_srcBL.resize(nPanels());
    fill(m_srcBL.begin(), m_srcBL.end(), 0);
    fill(m_dstar.begin(), m_dstar.end(), 0);
}


bool Stream2d::solve()
{
    // build and inverse the matrix
    // matrix inverse is required to compute the source strength influences

    makeAij();

    m_bLinearSolution = matrix::inverseMatrixLapack(m_matsize, m_aij, std::thread::hardware_concurrency());
    if(!m_bLinearSolution)
    {
        std::fill(m_gam0.begin(),  m_gam0.end(),  0);
        std::fill(m_gam90.begin(), m_gam90.end(), 0);
        m_Psi = 0.0;
        return false;
    }

    float Q = 1.0f;
    std::vector<double> rhs0(m_matsize), rhs90(m_matsize);
    makeRHS(rhs0,   0.0, Q);
    makeRHS(rhs90, 90.0, Q);

    // solve the linear system for the vorticities
    m_gam0.resize(m_matsize-1);  // no need for the stream function value
    m_gam90.resize(m_matsize-1);  // no need for the stream function value

    for(uint i=0; i<m_gam0.size(); i++)
    {
        m_gam0[i] = m_gam90[i] = 0.0;
        for(uint j=0; j<m_gam0.size(); j++)
        {
            m_gam0[i]  += m_aij[i*m_matsize + j] * rhs0[j];
            m_gam90[i] += m_aij[i*m_matsize + j] * rhs90[j];
        }
    }

    // get the viscous contribution to vorticities
    // The influence of source panels is converted to equivalent vorticities at nodes.
    // See Drela XFoil eq. (10)
    makeBpij();

    return true;
}


bool Stream2d::calcSolution(double alpha, double Q)
{
    m_gamma_inv.resize(m_matsize-1);

    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);
    for(uint i=0; i<m_gam0.size(); i++)
    {
        m_gamma_inv[i] = m_gam0[i]*cosa + m_gam90[i]*sina;
        m_gamma_inv[i] *= Q;
    }

    // get the viscous contribution to vorticities

    double *gamm = new double(m_matsize);
    memset(gamm, 0, m_matsize*sizeof(double));

    bool bTrace = false;

    if(bTrace)
    {
        std::string out;
        matrix::display_mat(m_bpij.data(), m_matsize, int(m_Panel.size()), out);
        matrix::display_vec(m_srcBL.data(), int(m_Panel.size()), out);
        qDebug("%s", out.c_str());
    }

//    for(int ip=0; ip<m_Panel.size(); ip++)        qDebug(" %4d    %g", ip, m_Panel.at(ip).length());

    matrix::matVecMultLapack(m_bpij.data(), m_srcBL.data(), gamm, m_matsize, int(m_Panel.size()));

    for(uint i=0; i<m_gamma_src.size(); i++)
        m_gamma_src[i] = gamm[i];

    delete gamm;

    return true;
}


void Stream2d::makeRHS(std::vector<double> &rhs, double alpha, double Q)
{
    int nNodes = m_Foil.nNodes();

    Vector2d VInf(Q*cos(alpha*PI/180.0), Q*sin(alpha*PI/180.0));

    //iterate on all nodes
    for(int i=0; i<nNodes; i++)
    {
        rhs[i] = -VInf.x*m_Foil.node(i).y + VInf.y*m_Foil.node(i).x;
    }

    // add Kutta condition on the last line
    rhs[m_matsize-1] = 0.0;  // the wake carries no load : mu_0 + mu_n-1 = 0.0
}


/** Makes the source influence matrix b'ij defined in Drela 1989 eq. (10) */
void Stream2d::makeBpij()
{
    const int N = m_Foil.nNodes();
    const int Np = nPanels(); // = N foil nodes + nw wake nodes
    const int nrows = N+1; // =m_matsize;

    std::vector<double> bij(nrows*Np, 0); //  matrix of stream influence of panel p at node n;

    // make -bij to enable fast matmult with aij-1
    makeBij(bij, -1.0);
    m_bpij.resize(nrows*Np);
    std::fill(m_bpij.begin(), m_bpij.end(), 0);

/*    bool bTrace = false;
    if(bTrace)
    {
        display_mat(m_aij, m_matsize);
        qDebug();
        display_mat(bij.data(), nrows, Np1);
        qDebug();
    }*/

    // Drela eq. (10):  bpij = - aij-¹ x bij
    // at this stage, m_aij holds the inverse of the vortex influence matrix
    matrix::matMultLAPACK(m_aij.data(), bij.data(), m_bpij.data(), nrows, nrows, Np);

//    if(bTrace)display_mat(m_bpij.data(), nrows, Np1);

}


void Stream2d::makeBij(std::vector<double> &bij, double sign)
{
    const int nNodes = m_Foil.nNodes();
    const int nCols = nPanels();
    double psi(0);

    // build the matrix in rows
    for(int in=0; in<nNodes; in++)
    {
        // evaluate the stream function at node i
        Node2d n2d = m_Foil.node(in);

        // make the influences of panel j at node i
        for (int j=0; j<nPanels(); j++)
        {
            Panel2d const &p2d = m_Panel.at(j);
            if(in!=p2d.A.index() && in!=p2d.B.index())
            {
                // if the node isn't one of the panel's endpoint, straigthforward evaluation of the stream function
                psi=0;
                p2d.uniformSource(n2d, nullptr, &psi, nullptr);
                bij[in*nCols+j] = psi*sign;
            }
            else
            {
                psi=0;
//                p2d.uniformSource(n2d + p2d.normal()*0.001, nullptr, &psi, nullptr);
                p2d.uniformSource(n2d, nullptr, &psi, nullptr);
                bij[in*nCols+j] = psi*sign;
            }
        }
    }
}


/**
 * Stream function formulation
 * This method builds two nearly identical matrices
 *   aij is the NxN vortex influence matrix defined in XFoil eq. (7)
 *   mat is the full (N+1)x(N+1) matrix for the linear problem, which is the aij matrix
 *   augmented with
 *       one row for the Kutta condition (XFoil eq. (8) or (9))
 *       one column for the unknown stream function value psi_0 on the body
 * Matrix mat is needed to solve the linear system
 * Matrix aij is needed to transform the source strengths into panel vorticities using XFoil eq. (10)
 */
void Stream2d::makeAij()
{
    int nNodes = m_Foil.nNodes();
    bool bSharp = m_Foil.sharpTE();

    //define TE panel geometry i.a.w. Drela fig.1

    std::fill(m_aij.begin(), m_aij.end(), 0);
    double psi_p(0), psi_m(0), psi_sigma(0);

    //iterate on all nodes
    for(int i=0; i<m_Foil.nNodes(); i++)
    {
        //evaluate the stream function at the edge of the displacement thickness normal to node i
        Node2d n2d = m_Foil.node(i);
//        n2d += n2d.N()*m_dstar.at(i);
//        n2d += n2d.N()*0.0001;

        // get all panel contributions, number of nodes-1 unless there is a blunt TE panel
        for (int j=0; j<m_Foil.nNodes()-1; j++)
        {
            //coefficient aij is the influence of panel j at node i
            psi_p = psi_m = 0.0;
            m_Panel.at(j).linearVortex(n2d, psi_p, psi_m);

            m_aij[i*m_matsize+j]   += psi_p-psi_m;      // gamma_j   contribution
            m_aij[i*m_matsize+j+1] += psi_p+psi_m;      // gamma_j+1 contribution
        }

        //add psi_0 column component in the Nth (last) column
        m_aij[i*m_matsize+ m_matsize-1] = -1.0;

        // Special treatment for the panel linking last and first nodes
        // Drela eq. 3 - panel N is the one linking last node to first node across TE
        if(!bSharp)
        {
            m_TEPanel.linearVortex(n2d, psi_p, psi_m);
            m_TEPanel.uniformSource(n2d, nullptr, &psi_sigma, nullptr);

            // note psi_sig includes the 1/2pi factor omitted in Drela eq. 6
            // extra 1/2 factor on psi_sigma contrib to be identical to xfoil's implementation
            double teContribution =  1.0/2.0 *(psi_sigma * m_sigTE)
                                   + psi_p * m_gamTE;
//teContribution=0.0;
            m_aij[i*m_matsize]              +=  teContribution;
            m_aij[i*m_matsize+ m_matsize-2] += -teContribution;
        }
    }

    if(bSharp)
    {
        // If the TE is sharp, node 0 and n-1 coincide, hence lines 0 and n-1 are identical
        // We replace the equation for node n-1 with a special condition
        // In Drela's paper, the recommendation is to force an average
        //     gam(3) - 2.gam(2) + gam(1) = gam(N-3) - 2.gam(N-2) + gam(N-1)
        // However, in the code itself, the condition seems to be to
        // "set velocity component along bisector line" - See function ggcalc()
        //
        memset(m_aij.data()+(m_matsize-2)*m_matsize, 0, ulong(m_matsize)*sizeof(double));

        // Drela eq.9
        m_aij[(m_matsize-2)*m_matsize  ] =  1.0;   //    gam(1)
        m_aij[(m_matsize-2)*m_matsize+1] = -2.0;   // -2.gam(2)
        m_aij[(m_matsize-2)*m_matsize+2] =  1.0;   //    gam(3)
        m_aij[(m_matsize-1)*m_matsize-4] = -1.0;   //    gam(N-3)
        m_aij[(m_matsize-1)*m_matsize-3] = +2.0;   // -2.gam(N-2)
        m_aij[(m_matsize-1)*m_matsize-2] = -1.0;   //    gam(N-1)
    }

    // add Kutta condition on the last line:
    //equality of top and bottom velocities <-> equality of top and bottom circulations
    m_aij[nNodes*m_matsize + 0]        = 1.0;
    m_aij[nNodes*m_matsize + nNodes-1] = 1.0;

//    display_mat(m_aij.data(), m_matsize);
}


void Stream2d::setFoil(Foil const *pFoil)
{
    if(!pFoil) return;

    m_Foil.copy(pFoil);
    // redundant?
    /*    m_Foil.initGeometry();
    m_Foil.makeCubicSpline();
    m_Foil.makeNormalsFromCubic();*/

    makeFoilPanels(&m_Foil);
    allocateAnalysisArrays(m_Foil.nNodes());

    m_bLinearSolution = false;

    setTEDensities();
}


void Stream2d::resizeSourceArrays()
{
    m_srcBL.resize(nPanels());
    std::fill(m_srcBL.begin(), m_srcBL.end(), 0);
}


void Stream2d::setTEDensities()
{
    Vector2d s = m_Foil.TEbisector();
    Vector2d t = m_TEPanel.T;
    t.normalize();

    m_sigTE = fabs(s.x*t.y-s.y*t.x);
    m_gamTE = fabs(s.dot(t));
}


void Stream2d::allocateAnalysisArrays(int nPts)
{
    // set matrix size
    m_matsize = nPts+1; // +1 for stream function value psi_0, and for the Kutta condition, eq. (8)

    m_gamma_inv.resize(nPts);
    std::fill(m_gamma_inv.begin(), m_gamma_inv.end(), 0);

    m_gam0.resize(nPts);
    m_gam90.resize(nPts);
    std::fill(m_gam0.begin(),  m_gam0.end(), 0);
    std::fill(m_gam90.begin(), m_gam90.end(), 0);

    m_gamma_src.resize(nPts);
    std::fill(m_gamma_src.begin(), m_gamma_src.end(), 0);

    m_dstar.resize(nPts);
    std::fill(m_dstar.begin(), m_dstar.end(), 0);

    // make a temp array of source strengths limited to the foil
    // to enable first wake calculation;
    // will be extended once the wake is defined
    m_srcBL.resize(nPanels());
    std::fill(m_srcBL.begin(), m_srcBL.end(), 0);

    // construct and initialize arrays
    m_RHS.resize(m_matsize);
    std::fill(m_RHS.begin(), m_RHS.end(), 0);

    m_aij.resize(m_matsize*m_matsize);
    std::fill(m_aij.begin(), m_aij.end(), 0);
}


/**
 * @brief Returns the velocity induced at point pt p by the vorticity distribution.
 * @param pt the point where the velocity is to be evaluated
 * @param vel the calculated velocity
 */
void Stream2d::getVelocity(double alpha, double qinf, Vector2d const & pt, Vector2d &vel, bool bSigma) const
{
    Vector2d V, dVdg1, dVdg2;
    Vector2d VSigma;

    double dl=0.00001;
    double psi_p=0, psi_m=0;
    double psi_0=0, psi_x=0, psi_y=0;
    double gp(0), gp1(0);
    bool bDiff = false;
    vel.set(qinf*cos(alpha*PI/180.0), qinf*sin(alpha*PI/180.0));

    for(int p=0; p<m_Foil.nNodes()-1; p++)
    {
        Panel2d const &p2 = m_Panel.at(p);
        if(bDiff)
        {
//            Using finite differences
            double gp  = gamma(p);
            double gp1 = gamma(p+1);
            p2.linearVortex(pt, psi_p, psi_m);
            psi_0 =   psi_p*(gp+gp1) + psi_m*(gp1-gp);

            p2.linearVortex(Vector2d(pt.x+dl,pt.y), psi_p, psi_m);
            psi_x = psi_p*(gp+gp1) + psi_m*(gp1-gp);

            p2.linearVortex(Vector2d(pt.x,pt.y+dl), psi_p, psi_m);
            psi_y = psi_p*(gp+gp1) + psi_m*(gp1-gp);

            V.x = -(psi_y-psi_0)/dl; /** @todo check reason for opposite sign */
            V.y =  (psi_x-psi_0)/dl;
        }
        else
        {
            p2.linearVortex(pt, &dVdg1, &dVdg2);
            gp  = m_gamma_inv.at(p)  +m_gamma_src.at(p);
            gp1 = m_gamma_inv.at(p+1)+m_gamma_src.at(p+1);
            V = dVdg1*(gp) + dVdg2*(gp1);
        }
        vel += V;
    }

    if(!m_Foil.sharpTE())
    {
        double dg = (m_gamma_inv.front()-m_gamma_inv.back())/2.0;


        m_TEPanel.linearVortex(pt, &dVdg1, &dVdg2);
        vel += (dVdg1 + dVdg2) *m_gamTE*dg;

        Vector2d VSigma;
        m_TEPanel.uniformSource(pt, nullptr, nullptr, &VSigma);
        vel += VSigma * m_sigTE*1.0/2.0 *dg;
    }


    if(bSigma)
    {
        for(uint ip=0; ip<m_Panel.size(); ip++)
        {
            Panel2d const &p2 = m_Panel.at(ip);
            p2.uniformSource(pt, nullptr, nullptr, &VSigma);
            vel += VSigma * m_srcBL.at(ip);
        }
    }
}


double Stream2d::streamValue(double alpha, double qinf, Vector2d const &pt) const
{
    double psi = 0.0;
    // freestream potential
    Vector2d VInf(qinf*cos(alpha*PI/180.0), qinf*sin(alpha*PI/180.0));
    psi = VInf.x*pt.y - VInf.y*pt.x;

    // vortex contributions
    double psi_p=0.0, psi_m=0.0;
    for(int p=0; p<m_Foil.nNodes()-1; p++)
    {
        m_Panel.at(p).linearVortex(pt, psi_p, psi_m);
        psi += (psi_p-psi_m)*(m_gamma_inv[p]  +m_gamma_src[p]  );
        psi += (psi_p+psi_m)*(m_gamma_inv[p+1]+m_gamma_src[p+1]);
    }

    // source contributions
    double psiSrc=0.0;
    for(uint p=0; p<m_Panel.size(); p++)
    {
        Panel2d const &p2d = m_Panel.at(p);
        p2d.uniformSource(pt, nullptr, &psiSrc, nullptr);
        psi += psiSrc*m_srcBL.at(p);
    }

    // TE contribution
    if(!m_Foil.sharpTE())
    {
        double psi_sigma = 0.0;

        m_TEPanel.linearVortex(pt, psi_p, psi_m);
        m_TEPanel.uniformSource(pt, nullptr, &psi_sigma, nullptr);

        // note psi_sig includes the 1/2pi factor omitted in Drela eq. 6
        // extra 1/2 factor on psi_sigma contrib to be identical to xfoil's implementation
        double teContribution =  1.0/2.0 *(psi_sigma * m_sigTE) + (psi_p * m_gamTE);
        teContribution *= (m_gamma_inv.front()  +m_gamma_src.front()) - (m_gamma_inv.back() +m_gamma_src.back());
        psi += teContribution;
    }
    return psi;
}


int Stream2d::getLEindex(double , double ) const
{
    int iLE=1;
    for(; iLE<int(m_Foil.nNodes()); iLE++)
    {
        if(m_gamma_inv[iLE]<0)
            return iLE;
    }
    return -1;
}


/*
double Stream2d::Cp(int iNode) const
{
    return 1.0-gamma(iNode)*gamma(iNode)/m_Q/m_Q;
}*/


double Stream2d::gamma(int iNode) const
{
    return m_gamma_inv.at(iNode)+m_gamma_src.at(iNode);
}


double Stream2d::surfaceVelocity(double alpha, double qinf, int iNode) const
{
    Vector2d vel;
    Node2d n2d = m_Node.at(iNode);
    n2d += n2d.normal()*OFFSET;
    getVelocity(alpha, qinf, n2d, vel);
    return vel.dot(n2d.T());
//    return gamma(iNode);
}


double Stream2d::getZeroLiftAngle()
{
    // make lift for 0° and 90°
    double l0=0, l90=0;

    for(int i=0; i<m_Foil.nNodes()-2; i++)
    {
        double ds0 = m_Panel[i].length();
        double ds1 = m_Panel[i+1].length();
        l0  += (ds0+ds1) * m_gam0[i+1];
        l90 += (ds0+ds1) * m_gam90[i+1];
    }

    double Alpha0 = -atan2(l0, l90) * 180.0/PI;
    return Alpha0;
}


double Stream2d::Cpv(double alpha, double qinf, int iNode) const
{
    Node2d n2d = node2d(iNode);
    n2d += node2d(iNode).normal()*OFFSET;
    Vector2d vel;
    getVelocity(alpha, qinf, n2d, vel);
    return 1.0-vel.norm()*vel.norm()/qinf/qinf;
}


void Stream2d::clearWakePanels()
{
    for(int i=nPanels()-1; i>=0; i--)
    {
        if(m_Panel.at(i).isWakePanel())
            m_Panel.erase(m_Panel.begin() + i);
    }
}


/** make source strengths from displacement thickness */
void Stream2d::makeSigma(std::vector<float> const&dstar, float ue, int iLE)
{
    float ds(0);
    float md0(0), md1(0);

    for(int i=iLE-1; i>=0; i--)
    {
        if(i<int(dstar.size()))
        {
            ds = (m_Node.at(i)-m_Node.at(i+1)).norm();
            md1   = ue*dstar.at(i);
            m_srcBL[i]  = (md1- md0)/ ds;
            md0 = md1;
        }
        else
            m_srcBL[i] = 0.0;
    }

    m_srcBL[iLE] = 0.0;
    md0 = 0.0;
    for(uint i=iLE+1; i<m_Node.size(); i++)
    {
        if(i<dstar.size())
        {
            ds = (m_Node.at(i)-m_Node.at(i-1)).norm();
            md1   = ue*dstar.at(i);
            m_srcBL[i-1]  = (md1- md0)/ ds;
            md0 = md1;
        }
        else
            m_srcBL[i-1] = 0.0;
    }
}


int Stream2d::firstWakePanelIndex() const
{
    int iwp = 0;
    for(iwp=0; iwp<nPanels(); iwp++)
    {
        if(panel2d(iwp).isWakePanel()) return iwp;
    }
    return -1; // need to return something
}


/** Wake panels are built in the streamwise direction with normals pointing upwards. */
void Stream2d::makeWakePanels(double alpha, double qinf, float xmax)
{
    // remove all existing wake panels and nodes
    for(int in=int(m_Node.size()-1); in>=0; in--)
    {
        if(m_Node.at(in).isWakeNode())
            m_Node.erase(m_Node.begin() + in);
    }

    for(int ip=nPanels()-1; ip>=0; ip--)
    {
        if(m_Panel.at(ip).isWakePanel())
            m_Panel.erase(m_Panel.begin() + ip);
    }

    // calculate a streamline from the TE

    Node2d firstwakenode;
    firstwakenode.x = (m_Foil.frontNode().x + m_Foil.backNode().x)/2.0;
    firstwakenode.y = (m_Foil.frontNode().y + m_Foil.backNode().y)/2.0;

    Vector2d bis = m_Foil.TEbisector();
    firstwakenode.setNormal(-bis.y, +bis.x); // wake node normals point upwards

    int nTE = m_Foil.nNodes()-1;

    Vector2d vel;
    Node2d pt;

    // Make a panel between the airfoil's TE and the first wake node
    // The contribution of this panel will be ignored (?), but it preserves
    // the continuity of indexes between nodes and panels
    firstwakenode.x += 0.001; // start with a slight offset to ensure that the wake develops in the streamwise direction

    // Set the index to -1, to ensure that there will be no source strength between TE
    // and first wake panel. This eliminates numerical issues.
    firstwakenode.setIndex(-1);
    firstwakenode.setWakeNode(true);

    double l = s_FirstWakePanelLength;
    if(s_bAdjustFirstWakePanel)
    {
        // make the first wake panel the same size as the last airfoil panel
        double lfirst = (m_Foil.node(1)-firstwakenode).norm();
        int penultimate = m_Foil.nNodes()-2;
        double llast = (m_Foil.node(penultimate)-firstwakenode).norm();
        l = (lfirst+llast)/2.0;
    }

    vel = m_Foil.TEbisector().normalized();
    vel *= l;

    int i=0;
    pt = firstwakenode;
    do
    {
        pt += vel;
        if(pt.x>xmax) break;

        pt.setWakeNode(true);
        pt.setIndex(int(m_Node.size()));
        vel.normalize();
        pt.setT(vel);
        m_Node.push_back(pt);

        if(i==0) pt.setT(vel.normalized());

        l *= s_WakeProgressionFactor;

        //get velocity for the construction of the next point
        getVelocity(alpha, qinf, pt, vel, false); /// @todo sharp panel and singular velocity
        vel.normalize(); // make this the tangent vector at the previous node
        vel *= l;

        i++;
    } while(pt.x<1.0+s_WakeLength && i<100); // limit to 100 wake panels in case of incorrect user input

    vel.normalize(); // make this the tangent vector at the back node
    m_Node.back().setT(vel);

    // push the link panel on the panel stack
    Panel2d linkpanel({firstwakenode, node2d(nTE+1)});
    linkpanel.setWakePanel(true);
    m_Panel.push_back(linkpanel);
    m_Panel.back().setIndex(int(m_Panel.size()-1));

    // build the wake panels
    int nwake = 0;
    for(int inode=nTE+1; inode<nNodes()-1; inode++)
    {
        // build the panel so that T points streamwise and that there is no
        // source stream function discontinuity upstream

        m_Panel.push_back({node2d(inode), node2d(inode+1)});
        Panel2d &p2d = m_Panel.back();
        p2d.setWakePanel(true);
        p2d.setIndex(int(m_Panel.size()-1));
        nwake++;
    }
    (void)nwake;
}


Foil* Stream2d::makeBlasiusSigma(double alpha, double qinf, double coef, bool bMakeFoil)
{
    int iLE = getLEindex(alpha, qinf);

    //make the source strengths for a Blasius displacement thickness
    double l=0, ds=0;

    int N = nNodes();
    int np = nPanels();

    std::vector<double> sig(np, 0), Ix(np,0);

    std::vector<double> md(N,0);
    m_dstar.resize(N);
    std::fill(m_dstar.begin(), m_dstar.end(), 0);
    std::fill(m_srcBL.begin(), m_srcBL.end(), 0);

    Vector2d Vel;
    md[iLE] = 0.0;
    l=0.0;
    for(int i=iLE-1; i>=0; i--)
    {
        ds = (m_Foil.node(i)-m_Foil.node(i+1)).norm();
        l += ds;
        double Re = l*qinf/1.e-5;

        m_dstar[i]  = 1.721 * l/sqrt(Re) * coef;

        Node2d n2d = node2d(i);
        n2d += n2d.normal()*m_dstar[i];
        getVelocity(alpha, qinf, n2d, Vel, false);
        float ue = Vel.dot(n2d.T());

        md[i]   = ue*m_dstar[i];
        sig[i]  = (md[i]- md[i+1])/ ds;
    }

    l=0.0;
    for(int i=iLE+1; i<nNodes(); i++)
    {
        ds = (node2d(i)-node2d(i-1)).norm();
        l += ds;
        double Re = l*qinf/1.e-5;

        m_dstar[i]  = 1.721 * l/sqrt(Re) * coef;

        Node2d n2d = node2d(i);
        n2d += n2d.normal()*m_dstar[i];
        getVelocity(alpha, qinf, n2d, Vel, false);
        float ue = Vel.dot(n2d.T());
        //T points forward on bottom nodes --> inverse ue
        if(n2d.isAirfoilNode()) ue=-ue;

        md[i]    = ue*m_dstar[i];
        sig[i-1] = (md[i]- md[i-1])/ ds;
    }


    for(uint ip=0; ip<sig.size(); ip++)
    {
        Ix[ip] = m_Panel[ip].I.x;
    }

    bool bSmoothed = false;
    if(bSmoothed) sgsmooth::smooth_nonuniform(4, 8, Ix, sig, m_srcBL);
    else  m_srcBL = sig;

    //    qDebug("   src  src_smoothed");
    //    for(int ip=0; ip<m_src.size(); ip++) qDebug("   %11g  %11g", sig[ip], m_src[ip]);

    // debug only: make a foil

    if(bMakeFoil)
    {
        Foil *pFoil = new Foil(&m_Foil);
        QString strange = QString::asprintf("_%2f", alpha);
        pFoil->setName("Blasius"+(strange+DEGch).toStdString());
            pFoil->setLineStipple(Line::DASH);
        pFoil->setVisible(true);

        for(uint in=0; in<m_Node.size(); in++)
        {
            Node2d n2d = m_Node[in];
            if(n2d.isWakeNode()) break;
            n2d += n2d.normal() * m_dstar[in];

            pFoil->setBasePoint(in, n2d);
        }

        pFoil->initGeometry();
        return pFoil;
    }

    return nullptr;
}


void Stream2d::setAirfoilSourceStrengths(float src)
{
    for(uint ip=0; ip<m_Panel.size(); ip++)
    {
        if(m_Panel.at(ip).isAirfoilPanel())
            m_srcBL[ip] = src;
    }
}


void Stream2d::setWakeSourceStrengths(float src)
{
    for(uint ip=0; ip<m_Panel.size(); ip++)
    {
        if(m_Panel.at(ip).isWakePanel())
            m_srcBL[ip] = src;
    }
}


/** checks that the stream function has the same value at all airfoil nodes */
void Stream2d::checkSolution(double alpha, double qinf) const
{
    Vector2d VInf(qinf*cos(alpha*PI/180.0), qinf*sin(alpha*PI/180.0));

    std::vector<double> psicalc(m_Foil.nNodes(), 0);
    std::vector<double> psigam(m_Foil.nNodes(), 0);
    std::vector<double> psisig(m_Foil.nNodes(), 0);

    std::vector<double> psiinf(m_Foil.nNodes());
    for(int i=0; i<m_Foil.nNodes(); i++)
    {
        psiinf[i] = +VInf.x*m_Foil.node(i).y - VInf.y*m_Foil.node(i).x;
    }

    //add all vortex contributions
    for(int i=0; i<m_Foil.nNodes(); i++)
    {
        double psi_p(0), psi_m(0);
        Node2d n2d = m_Foil.node(i); //evaluate the stream function at node i
        //        n2d += n2d.N()*m_dstar[i];
        //        n2d += n2d.N()*0.0001;

        // avoid endpoint singularities

        // get all panel contributions, number of nodes-1 unless there is a blunt TE panel
        for (int j=0; j<m_Foil.nNodes()-1; j++)
        {
            psi_p = psi_m = 0.0;
            m_Panel.at(j).linearVortex(n2d, psi_p, psi_m);

            psigam[i] += (psi_p-psi_m)*(gamma(j)  );
            psigam[i] += (psi_p+psi_m)*(gamma(j+1));
        }

        if(!m_Foil.sharpTE())
        {
            double psi_sigma = 0.0;

            m_TEPanel.linearVortex(n2d, psi_p, psi_m);
            m_TEPanel.uniformSource(n2d, nullptr, &psi_sigma, nullptr);

            // note psi_sig includes the 1/2pi factor omitted in Drela eq. 6
            // extra 1/2 factor on psi_sigma contrib to be identical to xfoil's implementation
            double teContribution =  1.0/2.0 *(psi_sigma * m_sigTE)
                                    + (psi_p * m_gamTE);
            teContribution *= (m_gamma_inv.front()  +m_gamma_src.front()) - (m_gamma_inv.back() +m_gamma_src.back());
            psigam[i] += teContribution;
        }
    }

    double psi=0;
    int Np = nPanels();
    for(int i=0; i<m_Foil.nNodes(); i++)
    {
        Node2d n2d = m_Foil.node(i); // evaluate the stream function at node i
        //        n2d += n2d.N()*m_dstar[i];
        //        n2d += n2d.N()*0.0001;
        psisig[i] = 0.0;
        // add all source panel contributions
        for (int j=0; j<Np; j++)
        {
            m_Panel.at(j).uniformSource(n2d, nullptr, &psi, nullptr);
            psisig[i] +=  psi * m_srcBL[j];
        }
    }

    // check that the stream function has a uniform value at nodes
    qDebug(" panel       psiqinf          psisig          psigam          psitot");
    for(uint i=0; i<psicalc.size(); i++)
    {
        psicalc[i] = psiinf[i] + psisig[i] + psigam [i]; // should be the same at all nodes
        qDebug("  %3d   %13.7g   %13.7g   %13.7g   %13.7g", i, psiinf[i], psisig[i], psigam[i], psicalc[i]);
    }

    // check normal velocity at nodes
    /*    Vector2d vel;
    for(int i=0; i<m_Foil.n(); i++)
    {
        Node2d n2d = m_Foil.node(i);
        n2d += n2d.N()*2.e-6; // slight offset to avoid singularities - slightly more than LENGTHPRECISION
        getVelocity(n2d, vel);
        qDebug(" %3d  %11g  %11g  %11g  %11g", i, n2d.x, n2d.y, vel.dot(n2d.N()), vel.dot(n2d.T()));
    }*/
}


void Stream2d::listCirculations() const
{
    qDebug("  Node       gma_inv         gam_src");
    for(int i=0; i<m_Foil.nNodes(); i++)
        qDebug("%4d    %13g    %13g ", i, m_gamma_inv.at(i), m_gamma_src.at(i));
}





