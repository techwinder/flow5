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


struct Attenuation
{
        float m_Constant{1.0}, m_Linear{0.5}, m_Quadratic{0};
};


struct Light
{
    public:
        void setDefaults(double reflength)
        {
            m_Red   = 1.0f;
            m_Green = 1.0f;
            m_Blue  = 1.0f;

            m_Ambient      = 0.3f;
            m_Diffuse      = 1.4f;
            m_Specular     = 0.7f;

            m_X   =  0.05f * float(reflength);
            m_Y   =  0.15f * float(reflength);
            m_Z   =  0.25f * float(reflength);

            m_EyeDist =  1.0f * float(reflength);

            m_iShininess = 10;

            m_Attenuation.m_Constant  = 1.0;
            m_Attenuation.m_Linear    = 0.5;
            m_Attenuation.m_Quadratic = 0.0;

            m_bIsLightOn = true;
        }

    public:

        float m_Ambient{0.3f}, m_Diffuse{1.2f}, m_Specular{0.5f}; // the light intensities
        float m_Red{1.0f}, m_Green{1.0f}, m_Blue{1.0f}; // the color of light
        float m_X{0.1f}, m_Y{0.3f}, m_Z{0.5f}; // coordinates in camera space
        bool m_bIsLightOn{true};

        float m_EyeDist{10.0}; // eye distance; unused in orthographic view, except to calculate light effects

        Attenuation m_Attenuation;
        int m_iShininess{10};
};
