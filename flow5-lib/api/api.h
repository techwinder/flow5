/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <string>
#include <queue>

#include <fl5lib_global.h>
#include <gmesh_globals.h>

class Foil;
class Polar;
class Opp;
class Plane;
class PlaneXfl;
class PlanePolar;
class POpp;
class XFoilTask;


namespace globals
{
    extern std::queue<std::string> g_log;

    /**
     * @brief deleteObjects Removes all 2d and 3d objects from the internal arrays and deletes them.
     * This function __MUST__ be called on exit, otherwise will cause a memory leak
     */
    FL5LIB_EXPORT void deleteObjects();

    /**
     * @brief pushToLog appends a message to the log. Private.
     * @param msg the message to append
     */
    FL5LIB_EXPORT void pushToLog(std::string const &msg);

    /**
     * @brief clearLog clears the message stack
     */
    FL5LIB_EXPORT void clearLog();

    /**
     * @brief poplog removes the front message in the queue and returns it
     * @return removes the front message in the queue and returns it
     */
    FL5LIB_EXPORT std::string poplog();

}



namespace plane
{
    /**
     * @brief makeEmptyPlane Creates an empty xfl-type plane with no wings or fuselage and stores it in the internal array
     * @return a pointer to the created object
     */
    FL5LIB_EXPORT PlaneXfl *makeEmptyPlane();


    /** @todo duplicate with GMesherWt::onHandleMeshResults */
    FL5LIB_EXPORT void meshFuse(PlaneXfl *pPlaneXfl, std::vector<int>selectedWings, bool bThickSurfaces, gmesh::enumGmshAlgo algorithm, std::string &log, std::string prefix);

}

