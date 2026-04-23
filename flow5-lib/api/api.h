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
     * @brief saveFl5Project Saves all the data to a .fl5 project file.
     * Overwrites any existing file
     * @param path the path to the project file
     * @return true if the save operation was successful
     */
    FL5LIB_EXPORT bool saveFl5Project(std::string const &pathname);

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


namespace foil
{
    /**
     * @brief loadFoil Loads a foil from a file and stores the pointer to the created object in the array
     * @param pathname the path to the .dat file; must include the extension
     * @return a pointer to the foil instance if successfully loaded and stored, nullptr otherwise
     */
    FL5LIB_EXPORT Foil * loadFoil(std::string const &pathname);

    /**
     * @brief makeNacaFoil Makes and stores in the database a NACA 4 or digits airfoil
     * @param digits 4 or 5 digits defining a NACA foil
     * @param name the name to give to the foil
     * @return true if successful, false otherwise. A false return value likely indicates incorrect digits.
     */
    FL5LIB_EXPORT Foil *makeNacaFoil(int digits, const std::string &name);

    /**
     * @brief getFoil Returns a pointer to the foil object with the given name, or a nullptr if none is found
     * @param name the foil's name
     * @return a pointer to the foil instance
     */
    FL5LIB_EXPORT Foil* foil(const std::string &name);

    /**
     * @brief createAnalysis Creates a generic 2d analysis, i.e. a polar, and associates it to the foil.
     * The analysis data can then be set by accessing the polar's class public  methods;
     * @param foilname the foil for which this analysis is to be created.
     * @return a pointer to the polar if creation was successful, nullptr otherwise.
     */
    FL5LIB_EXPORT Polar *createAnalysis(std::string const &foilname);


    /**
     * @brief importAnalysisFromXml reads an xml file containg the description of a 2d analysis
     * If sucessful, creates a Polar, stores it in the database and retuns a pointer to the object
     * @param pathname the path to the .xml file,
     * @return a pointer to the polar if successfully created, nullptr otherwise
     */
    FL5LIB_EXPORT Polar *importAnalysisFromXml(std::string const &pathname);

}

namespace plane
{
    /**
     * @brief makeEmptyPlane Creates an empty xfl-type plane with no wings or fuselage and stores it in the internal array
     * @return a pointer to the created object
     */
    FL5LIB_EXPORT PlaneXfl *makeEmptyPlane();

}

