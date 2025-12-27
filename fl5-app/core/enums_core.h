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


namespace xfl
{

    /** @enum The different application modules used in MainFrame*/
    enum enumApp {NOAPP, XDIRECT, XPLANE, XSAIL};

    /**< @enum The different image formats usable to export screen captures*/
    enum enumImageFormat {PNG, JPEG, BMP};

    /** @enum The different number of graphs in the polar view */
    enum enumGraphView {ONEGRAPH, TWOGRAPHS, FOURGRAPHS, ALLGRAPHS, NOGRAPH};


    /** used for GUI conversions to/from strings */
    enum enumDataType {BOOLVALUE, INTEGER, DOUBLEVALUE, STRING, PANELDISTRIBUTION, FOILNAME, BODYTYPE, FUSEDRAG,
                       POLARTYPE, ANALYSISMETHOD, REFDIMENSIONS, WINGTYPE, BOUNDARYCONDITION};


}


