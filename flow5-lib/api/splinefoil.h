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



/**
 *@file This class FL5LIB_EXPORT defines the splined foil object used in foil design.
 */


#pragma once

#include <vector>

#include <bspline.h>
#include <vector2d.h>


class Foil;

/**
*@class FL5LIB_EXPORT SplineFoil
*@brief  The class FL5LIB_EXPORT which defines the splined foil object.

The foil is contructed based on one spline for the upper surface and one spline for the lower surface.
*/
class FL5LIB_EXPORT SplineFoil
{
    public:
        SplineFoil();
        SplineFoil(const SplineFoil *pSF);

        bool isSymmetric()    const {return m_bSymmetric;}
        void setSymmetric(bool b) {m_bSymmetric=b;}

        bool bOutPoints()     const {return m_bOutPoints;}
        bool bCenterLine()    const {return m_bCenterLine;}
        void showCenterLine(bool bShow) {m_bCenterLine = bShow;}

        void setTheStyle(LineStyle ls) {m_theStyle=ls; m_Extrados.setTheStyle(ls); m_Intrados.setTheStyle(ls);}
        LineStyle theStyle() const {return m_theStyle;}

        bool isVisible() const {return m_theStyle.m_bIsVisible;}
        void setVisible(bool bVisible) {m_theStyle.m_bIsVisible = bVisible; m_Extrados.setVisible(bVisible); m_Intrados.setVisible(bVisible);}

        Line::enumLineStipple stipple() const {return m_theStyle.m_Stipple;}
        void setStipple(Line::enumLineStipple style){m_theStyle.m_Stipple = style; m_Extrados.setStipple(style); m_Intrados.setStipple(style);}

        int width() const {return m_theStyle.m_Width;}
        void setWidth(int width){m_theStyle.m_Width = width; m_Extrados.setWidth(width); m_Intrados.setWidth(width);}

        void setColor(fl5Color const &clr) {m_theStyle.m_Color=clr;}
        fl5Color const &color() const {return m_theStyle.m_Color;}

        Line::enumPointStyle pointStyle()  const {return m_theStyle.m_Symbol;}
        void setPointStyle(Line::enumPointStyle ps) {m_theStyle.m_Symbol = ps; m_Extrados.setPointStyle(ps); m_Intrados.setPointStyle(ps);}

        bool isModified() const {return m_bModified;}
        void setModified(bool bModified){m_bModified = bModified;}

        void compMidLine();
        void makeSplineFoil();

        void initSplineFoil();

        bool serializeXfl(QDataStream &ar, bool bIsStoring);
        bool serializeFl5(QDataStream &ar, bool bIsStoring);

        void copy(const SplineFoil *pSF);
        void exportToFoil(Foil *pFoil) const;
        void setCurveParams(LineStyle const &ls);

        double camber()  const {return m_fCamber;}
        double xCamber() const {return m_fxCambMax;}
        double thickness() const {return m_fThickness;}
        double xThickness() const {return m_fxThickMax;}

        bool bClosedTE() const {return m_bForceCloseTE;}
        bool bClosedLE() const {return m_bForceCloseLE;}
        void setClosedTE(bool bClosed) {m_bForceCloseTE=bClosed;}
        void setClosedLE(bool bClosed) {m_bForceCloseLE=bClosed;}

        BSpline &intrados() {return m_Intrados;}
        BSpline &extrados() {return m_Extrados;}
        BSpline const &intrados() const {return m_Intrados;}
        BSpline const &extrados() const {return m_Extrados;}

        BSpline *spline(int is) {if(is==0) return &m_Intrados; else if(is==1) return &m_Extrados; else return nullptr;}

        std::vector<Vector2d> const &midPoints() const {return m_rpMid;}

    private:
        bool m_bModified;                /**< false if the SplineFoil has been serialized in its current dtate, false otherwise */
        bool m_bOutPoints;               /**< true if the ouput line points should be displayed */
        bool m_bCenterLine;              /**< true if the SplineFoil's mean camber line is to be displayed */

        // properties
        bool m_bSymmetric;                /**< true if the SplineFoil is symmetric. In which case the lower surface is set as symmetric of the upper surface. */
        bool m_bForceCloseLE;            /**< true if the leading end points of the top and bottom spline should be positioned at the same place */
        bool m_bForceCloseTE;            /**< true if the traling end points of the top and bottom spline should be positioned at the same place */

        LineStyle m_theStyle;


        double m_fCamber;                /**< the SplineFoil's max camber */
        double m_fThickness;             /**< the SplineFoil's max thickness */
        double m_fxCambMax;              /**< the x-position of the SplineFoil's max camber point */
        double m_fxThickMax;             /**< the x-position of the SplineFoil's max thickness point */
        BSpline m_Extrados;               /**< the spline which defines the upper surface */
        BSpline m_Intrados;               /**< the spline which defines the lower surface */
        std::vector<Vector2d> m_rpMid;  /**< the points on the SplineFoil's mid camber line @todo replace with a QVarLengthArray */
};

