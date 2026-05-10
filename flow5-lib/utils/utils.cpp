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

#include <fstream>
#include <iostream>
#include <iterator>
#include <format>


#if defined ACCELERATE_NEW_LAPACK
  #include <Accelerate/Accelerate.h>
  #define lapack_int int
#elif defined INTEL_MKL
    #include <mkl.h>
#elif defined OPENBLAS
    #include <openblas/lapack.h>
#endif


#include <utils.h>



fl5Color xfl::Orchid         = fl5Color(218,112,214);
fl5Color xfl::BlueViolet     = fl5Color(138,43,226);
fl5Color xfl::SteelBlue      = fl5Color(70,130,180);
fl5Color xfl::CornFlowerBlue = fl5Color(100,149,237);
fl5Color xfl::PhugoidGreen   = fl5Color(45,82,39);
fl5Color xfl::Bisque         = fl5Color(255,228,196);
fl5Color xfl::FireBrick      = fl5Color(178,34,34);
fl5Color xfl::LightCoral     = fl5Color(240,128,128);
fl5Color xfl::GreenYellow    = fl5Color(173,255,47);
fl5Color xfl::Magenta        = fl5Color(255,0,255);
fl5Color xfl::IndianRed      = fl5Color(205,92,92);
fl5Color xfl::Turquoise      = fl5Color(64,224,208);




/**
* Extracts nfloat values from a std::string, and returns the number of extracted values.
* @param nValues is the size of the val array
*/
int xfl::readValues(std::string const &theline, float val[], int nValues)
{
//    std::string line = theline;
//    trim(line);

    std::istringstream buffer(theline);
    std::vector<std::string> split;

    std::copy(std::istream_iterator<std::string>(buffer),
              std::istream_iterator<std::string>(),
              std::back_inserter(split));

    int nread = 0;


    std::string::size_type sz(0);

    try
    {
        for(unsigned int is=0; is<split.size() && nread<nValues; is++)
        {
            val[nread++] = std::stof(split.at(is), &sz);
        }
    }
    catch (const std::invalid_argument&)
    {
//          std::cerr << "Invalid argument: " << ia.what() << '\n';
    }
    catch (const std::out_of_range& )
    {
//        std::cerr << "Out of Range error: " << oor.what() << '\n';
    }
    catch(...)
    {
//        std::cerr << "Unknown error reading floats"<< '\n';
    }

    return nread;
}


float xfl::randomfloat(float fmax)
{
    float f = float(std::rand()) / float(RAND_MAX);
    return f * fmax;
}


int xfl::randomInt(int range)
{
    range++;
    return std::rand() % range;
}

/*
fl5Color xfl::randomObjectColor(bool )
{
    int R = randomInt(255);
    int G = randomInt(255);
    int B = randomInt(255);

    assert(R<256);
    assert(G<256);
    assert(B<256);

    return fl5Color(R,G,B);
}*/


float xfl::getRed(float tau)
{
    if     (tau>5.0f/6.0f) return 1.0f;
    else if(tau>4.0f/6.0f) return (6.0f*(tau-4.0f/6.0f));
    else if(tau>2.0f/6.0f) return 0.0f;
    else if(tau>1.0f/6.0f) return 1.0f - (6.0f*(tau-1.0f/6.0f));
    else                   return 1.0f;
}


float xfl::getGreen(float tau)
{
    if      (tau<2.0f/6.0f) return 0.0f;
    else if (tau<3.0f/6.0f) return 6.0f*(tau-2.0f/6.0f);
    else if (tau<5.0f/6.0f) return 1.0f;
    else if (tau<6.0f/6.0f) return 1.0f - (6.0f*(tau-5.0f/6.0f));
    else                    return 0.0f;
}


float xfl::getBlue(float tau)
{
    if      (tau<0.0f)      return 0.0f;
    else if (tau<1.0f/6.0f) return 6.0f * tau;
    else if (tau<3.0f/6.0f) return 1.0f;
    else if (tau<4.0f/6.0f) return 1.0f - (6.0f*(tau-3.0f/6.0f));
    else                    return 0.0f;
}


bool xfl::stringToFile(std::string const &string, std::string const &path)
{
    std::ofstream outstream;
    try
    {
        outstream.open(path);
        outstream << string;
        outstream.close();
    }
    catch (const std::ofstream::failure &)
    {
      return false;
    }
    return true;
}


bool xfl::stringFromFile(std::string &string, std::string const &path)
{
    std::ifstream instream;

    try {
      instream.open(path);
      std::stringstream buffer(string);
      buffer << instream.rdbuf();
//      string = buffer.str();
    }
    catch (const std::ifstream::failure &)
    {
      return false;
    }
    return true;
}



std::string xfl::MklVersion()
{
    std::string strange;

#ifdef INTEL_MKL
    MKLVersion Version;

    mkl_get_version(&Version);

    strange += std::format("<p><b>Version: </b>{:d}.{:d}.{:d}<br>", Version.MajorVersion, Version.MinorVersion, Version.UpdateVersion);
    strange += std::format("<b>Processor optimization: </b> {:s}", Version.Processor) + "</p>";
#endif
    return strange;
}


