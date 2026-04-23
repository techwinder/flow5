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

#include <frame.h>


#include <constants.h>


int Frame::s_iHighlight = -1;
int Frame::s_iSelect = -1;


/**
* The public constructor
* @param nCtrlPts the number of points with which the Frame is initialized.
*/
Frame::Frame(int nCtrlPts)
{
    m_Position.set(0.0,0.0,0.0);
    m_CtrlPoint.clear();
    for(int ic=0; ic<nCtrlPts; ic++)
    {
        m_CtrlPoint.push_back(Vector3d(0.0,0.0,0.0));
    }
    m_Ry = 0.0;
    m_bTip = false;
}


/**
*Identifies if an input point matches with one of the Frame's control points
*@param Point the input point
*@param ZoomFactor the scaling factor to be withdrawn from the Point prior to the comparison.
* @todo withdrawal to be performed from within the calling function.
*@return the index of the point in the array which matches with the input point
*/
int Frame::isPoint(const Vector3d &Point, double deltax, double deltay, double deltaz) const
{
    for(uint l=0; l<m_CtrlPoint.size(); l++)
    {
        if (fabs(Point.x-m_CtrlPoint[l].x)<deltax &&
            fabs(Point.y-m_CtrlPoint[l].y)<deltay &&
            fabs(Point.z-m_CtrlPoint[l].z)<deltaz)
              return l;
    }
    return -10;
}


/**
 * Loads or Saves the data of this spline to a binary file
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool Frame::serializeFrameXf7(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat=500001; //500001 : first xf7 format
    int nIntSpares(0);
    int nDbleSpares(0);
    int n(0);
    double dble(0);

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << m_Position.x << m_Position.y << m_Position.z;
        ar << m_Ry;
        ar << int(m_CtrlPoint.size());
        for(uint k=0; k<m_CtrlPoint.size(); k++)
        {
            ar << m_CtrlPoint[k].x << m_CtrlPoint[k].y << m_CtrlPoint[k].z;
        }
        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat!=500001) return false;
        ar >> m_Position.x >> m_Position.y >> m_Position.z;
        ar >> m_Ry;
        int nPts=0;
        ar >> nPts;
        m_CtrlPoint.clear();
        double dx(0), dy(0), dz(0);
        for(int k=0; k<nPts; k++)
        {
            ar >> dx >> dy >> dz;
            m_CtrlPoint.push_back({dx, dy, dz});
        }
        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}


bool Frame::serializeFrameXfl(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat(0);
    int k(0),n(0);
    float fx(0), fy(0), fz(0);

    if(bIsStoring)
    {
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<1000 || ArchiveFormat>1100) return false;
        ar >> n;
        m_CtrlPoint.clear();
        for(k=0; k<n; k++)
        {
            ar >> fx;
            ar >> fy;
            ar >> fz;
            m_CtrlPoint.push_back({fx, fy, fz});
        }
    }
    return true;
}

/**
* Removes a point from the array of control points.
*@param n the index of the control point to remove in the array
*@return true if the input index is within the array's boundaries, false otherwise
*/
bool Frame::removePoint(int n)
{
    if (n>=0 && n<int(m_CtrlPoint.size()))
    {
        m_CtrlPoint.erase(m_CtrlPoint.begin()+n);
        return true;
    }
    return false;
}


/**
*Inserts a new point at a specified index in the array of control points.
* the point is inserted at a mid position between the two adjacent points, or positioned 1/5 of hte distance of the last two points in the array.
*@param n the index at which a new points will be inserted
*/
void Frame::insertPoint(int n)
{
    m_CtrlPoint.insert(m_CtrlPoint.begin()+n, Vector3d(0.0,0.0,0.0));
    if(n==0)
    {
        m_CtrlPoint[n].x = m_CtrlPoint[n+1].x;
        m_CtrlPoint[n].y = m_CtrlPoint[n+1].y*1.1;
        m_CtrlPoint[n].z = m_CtrlPoint[n+1].z*1.1;
    }
    else if(n>0 && n<int(m_CtrlPoint.size())-1)
    {
        m_CtrlPoint[n] = (m_CtrlPoint[n+1] + m_CtrlPoint[n-1])/2.0;
    }
    else if(n==int(m_CtrlPoint.size())-1)
    {
        m_CtrlPoint[n].x = m_CtrlPoint[n-1].x;
        m_CtrlPoint[n].y = m_CtrlPoint[n-1].y*1.1;
        m_CtrlPoint[n].z = m_CtrlPoint[n-1].z*1.1;
    }
    s_iSelect = n;
}


/**
*Inserts a new point at a specified index in the array of control points.
* @param n the index at which a new points will be inserted
* @param Pt the coordinates of the point to insert
*/
void Frame::insertPoint(int n, Vector3d const& Pt)
{
    m_CtrlPoint.insert(m_CtrlPoint.begin()+n, Pt);
    s_iSelect = n;
}


/**
* Inserts a new point at a position in crescending order on the specified axis.
* @param Real the coordinates of the point to insert
* @param iAxis the axis used as the index key
*/
int Frame::insertPoint(const Vector3d &Real, int iAxis)
{
    uint k=0;
    if(iAxis==1)
    {
        if(Real.x>m_CtrlPoint.front().x)
        {
            for(k=0; k<m_CtrlPoint.size()-1; k++)
            {
                if(m_CtrlPoint[k].x<=Real.x && Real.x <m_CtrlPoint[k+1].x)
                {
                    break;
                }
            }
        }
        else k=-1;
    }

    else if(iAxis==2)
    {
        if(Real.y>m_CtrlPoint.front().y)
        {
            for(k=0; k<m_CtrlPoint.size()-1; k++)
            {
                if(m_CtrlPoint[k].y<=Real.y && Real.y <m_CtrlPoint[k+1].y)
                {
                    break;
                }
            }
        }
        else k=-1;
    }
    else if(iAxis==3)
    {
        double areal = atan2(Real.y, Real.z) *180/PI;

        for(k=0; k<m_CtrlPoint.size(); k++)
        {
            double actrlPt = atan2(m_CtrlPoint.at(k).y, m_CtrlPoint.at(k).z) *180/PI;
            if(areal<=actrlPt) break;
        }
        k--;
    }

    m_CtrlPoint.insert(m_CtrlPoint.begin() + k+1, Real);
    s_iSelect = k+1;
    return k+1;
}


/**
 * Returns the Frame's height as the difference of the z-coordinate of the last and first control points.
 *@return the Frame's height
 */
double Frame::height() const
{
    return (m_CtrlPoint.back() - m_CtrlPoint.front()).norm();
/*    double hmin    =  10.0;
    double hmax = -10.0;
    for(int k=0; k<m_CtrlPoint.size(); k++)
    {
        if(m_CtrlPoint[k].z<hmin) hmin = m_CtrlPoint[k].z;
        if(m_CtrlPoint[k].z>hmax) hmax = m_CtrlPoint[k].z;
    }
    return qAbs(hmax-hmin);*/
}


/**
 * Returns the Frame's z-position as the highest and lowest z-values in the array of control points.
 *@return the Frame's z-position
 */
double Frame::zPos() const
{
    double hmin    =  10.0;
    double hmax = -10.0;
    for(uint k=0; k<m_CtrlPoint.size(); k++)
    {
        if(m_CtrlPoint.at(k).z<hmin) hmin = m_CtrlPoint.at(k).z;
        if(m_CtrlPoint.at(k).z>hmax) hmax = m_CtrlPoint.at(k).z;
    }
    return (hmax+hmin)/2.0;
}


/**
 * Copies the data from an existing Frame
 * @param pFrame a pointer to the Frame object from which to copy the data
*/
void Frame::copyFrame(const Frame &frame)
{
    m_Position = frame.m_Position;
    m_Ry = frame.m_Ry;
    m_CtrlPoint = frame.m_CtrlPoint;
}


/**
 * Copies the control point data from an existing list of points
 * @param pPointList a pointer to the list of points
*/
void Frame::copyPoints(std::vector<Vector3d> const &PointList)
{
    m_CtrlPoint = PointList;
}


/**
* Appends a new point at the end of the current array
* @param Pt to point to append
*/
void Frame::appendPoint(Vector3d const& Pt)
{
    m_CtrlPoint.push_back(Pt);
}


/**
* Sets the Frame's absolute position
* @param Pos the new position
*/
void Frame::setPosition(Vector3d const &Pos)
{
    double zpos=0.0;
    if(m_CtrlPoint.size()==0) zpos = 0.0;
    else                        zpos = (m_CtrlPoint.front().z + m_CtrlPoint.back().z)/2.0;

    m_Position = Pos;
    for(uint ic=0; ic<m_CtrlPoint.size(); ic++)
    {
        m_CtrlPoint[ic].x  = Pos.x;
        m_CtrlPoint[ic].z += Pos.z - zpos;
    }
}


/**
* Set the frame's position on the x-axis
*/
void Frame::setuPosition(int uAxis)
{
    for(uint ic=0; ic<m_CtrlPoint.size(); ic++)
    {
        m_CtrlPoint[ic][uAxis] = m_Position[uAxis];
    }
}


/**
* Set the frame's position on the x-axis
* @param u the new x-position
*/
void Frame::setuPosition(int uAxis, double u)
{
    m_Position[uAxis] = u;
    for(uint ic=0; ic<m_CtrlPoint.size(); ic++)
    {
        m_CtrlPoint[ic][uAxis] = u;
    }
}


void Frame::translate(Vector3d const & T)
{
    m_Position += T;
    for(uint ic=0; ic<m_CtrlPoint.size(); ic++)
    {
        m_CtrlPoint[ic] += T;
    }
}


void Frame::scale(double ratio)
{
    for(uint ic=0; ic<m_CtrlPoint.size(); ic++)
    {
        m_CtrlPoint[ic] *= ratio;
    }
}


/**
* Rotates the Control points by a specified angle about the Frame's Oy axis
*@param Angle the rotation angle in degrees
*/
void Frame::rotateFrameY(double Angle)
{
    if(!m_CtrlPoint.size()) return;

//    Vector3d RotationCenter = m_CtrlPoint.front();
    for(uint ic=0; ic<m_CtrlPoint.size(); ic++)
    {
        m_CtrlPoint[ic].rotateY(m_Position, Angle);
    }
}


double Frame::developedLength() const
{
    double l=0.0;
    for(uint i=1; i<m_CtrlPoint.size(); i++)
    {
        l += sqrt( (m_CtrlPoint.at(i).y-(m_CtrlPoint.at(i-1).y)) * (m_CtrlPoint.at(i).y-(m_CtrlPoint.at(i-1).y))
                  +(m_CtrlPoint.at(i).z-(m_CtrlPoint.at(i-1).z)) * (m_CtrlPoint.at(i).z-(m_CtrlPoint.at(i-1).z)));
    }
    return l;
}


double Frame::segmentLength(int i) const
{
    if(i<0 || i>int(m_CtrlPoint.size())-1) return 0.0;

    return sqrt(  (m_CtrlPoint.at(i+1).y-(m_CtrlPoint.at(i).y)) * (m_CtrlPoint.at(i+1).y-(m_CtrlPoint.at(i).y))
                 +(m_CtrlPoint.at(i+1).z-(m_CtrlPoint.at(i).z)) * (m_CtrlPoint.at(i+1).z-(m_CtrlPoint.at(i).z)));

}

void Frame::resizeCtrlPoints(int nPoints)
{
    if(nPoints==nCtrlPoints()) return;
    if(nCtrlPoints()<2) return;

    // leave only first and last points
    m_CtrlPoint = {m_CtrlPoint.front(), m_CtrlPoint.back()};

    // rebuild as many points as needed between first and last
    int nNewPts = nPoints-nCtrlPoints();
    if(nNewPts<=0) return;

    Vector3d const &ss0 = m_CtrlPoint.front();
    Vector3d const &ssl = m_CtrlPoint.back();

    for(int ins=0; ins<nNewPts; ins++)
    {
        double ratio = double(ins+1)/double(nNewPts+1);
        m_CtrlPoint.insert(m_CtrlPoint.begin()+ins+1, ctrlPoint(ins));
        Vector3d &ns = m_CtrlPoint[ins+1];
        ns = ss0*(1-ratio) + ssl*ratio;
    }
}






