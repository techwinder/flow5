/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

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


