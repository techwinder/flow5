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

/** generic shader locations; not all locations are necessary nor defined for each shader */
struct ShaderLocations
{
    // Attribute data
    int m_attrVertex{-1}, m_attrNormal{-1};
    int m_attrColor{-1};
    int m_attrUV{-1}; // vertex attribute array containing the texture's UV coordinates
    int m_attrOffset{-1};

    // Uniforms
    int m_vmMatrix{-1}, m_pvmMatrix{-1};
    int m_Scale{-1}; // only used if instancing is enabled

    int m_Light{-1};
    int m_UniColor{-1};
    int m_ClipPlane{-1};

    int m_TwoSided{-1};

    int m_HasUniColor{-1};
    int m_HasTexture{-1};    // uniform defining whether a texture is enabled or not
    int m_IsInstanced{-1};

    int m_Pattern{-1}, m_nPatterns{-1};
    int m_Thickness{-1}, m_Viewport{-1};

    int m_State{-1};
    int m_Shape{-1};

    int m_TexSampler{-1}; // the id of the sampler; defaults to 0
};


/** flow compute shader locations */
struct FlowLocations
{
    int m_NPanels{-1};
    int m_Dt{-1};
    int m_VInf{-1};

    int m_TopLeft{-1}, m_BotRight{-1};

    int m_UniColor{-1};
    int m_HasUniColor{-1};
    int m_NVorton{-1};
    int m_VtnCoreSize{-1};
    int m_RK{-1};
    int m_HasGround{-1};
    int m_GroundHeight{-1};
};


