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

#include <apilog.h>


std::vector<std::string> Api::g_fl5Log;

void Api::clearLog()
{
    g_fl5Log.clear();
}


void Api::appendMessage(std::string const &msg)
{
    g_fl5Log.push_back(msg);
}


std::string Api::lastMessage()
{
    if(g_fl5Log.size()==0) return std::string();
    else return g_fl5Log.back();
}
