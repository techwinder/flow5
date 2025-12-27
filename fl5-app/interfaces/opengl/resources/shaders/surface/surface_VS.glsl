/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois 
    All rights reserved.

*****************************************************************************/

#version 330
// The surface vertex shader

in vec4 vertexPosition_modelSpace;
in vec3 vertexNormal_modelSpace;
in vec2 vertexUV;
in vec4 vertexColor;

in vec3  vertexOffset; // used if instanced

uniform mat4 pvmMatrix;
uniform mat4 vmMatrix;
uniform mat4 LightViewMatrix;
uniform int Instanced = 0;
uniform float uScale;  // used if instanced

// Output data; will be interpolated for each fragment.
out vec3 Position_viewSpace;
out vec3 Normal_viewSpace;
out vec2 UV;
out vec4 FragPosLightSpace;
out vec4 VSColor;

void main()
{
    vec4 vertexpos = vertexPosition_modelSpace;

    if(Instanced==1)
    {
        // scale first
        vertexpos.x = vertexPosition_modelSpace.x*uScale;
        vertexpos.y = vertexPosition_modelSpace.y*uScale;
        vertexpos.z = vertexPosition_modelSpace.z*uScale;
        vertexpos.w = vertexPosition_modelSpace.w;
        vertexpos += vec4(vertexOffset, 0.0);    // then translate
    }

    // Output position of the vertex, in clip space : MVP * position
    gl_Position =  pvmMatrix * vertexpos;

    vec4 vsPos = vmMatrix * vertexpos; // position of vertex in viewspace

    Position_viewSpace = vsPos.xyz;
    // Normal to the vertex, in camera space
    // Only correct if ModelMatrix does not scale the model! Use its inverse transpose if not.
    Normal_viewSpace = vec3(vmMatrix * vec4(vertexNormal_modelSpace,0));

    // the vertex color, optional
    VSColor = vertexColor;

    // in case there is a texture
    UV = vertexUV;

    // shadow addition
    FragPosLightSpace = LightViewMatrix * vertexpos;
}



