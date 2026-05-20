/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois 
    All rights reserved.

*****************************************************************************/

#version 330
// The surface fragment shader

#define PI 3.141592654f


in vec3 Position_viewSpace;
in vec3 Normal_viewSpace;
in vec4 VSColor;

uniform int HasUniColor = 0; // otherwise the attribute color will be used
uniform vec4 UniformColor;
uniform vec4 UniformColor2; // for gradients
uniform int HasTexture = 0;
uniform int HasGradient = 0;
uniform float GradientAngle = 0.0;
uniform int HasShadow = 0;
uniform int TwoSided;
uniform int LightOn;
uniform vec3 LightPosition_viewSpace;
uniform vec3 EyePosition_viewSpace;

uniform vec4 LightColor;
uniform float LightAmbient, LightDiffuse, LightSpecular;
uniform float Kc, Kl, Kq;
uniform float MaterialShininess;
uniform float clipPlane0; // defined in view-space

layout(location=0) out vec4 fragColor;



void main()
{
    if (Position_viewSpace.z > clipPlane0)
    {
        discard;
        return;
    }

    vec4 vertexcolor;
    if(HasUniColor==1) vertexcolor = UniformColor;
    else               vertexcolor = VSColor; // incoming from the Vertex or Geometry Shaders


    if(LightOn==1)
    {
        // Material properties
        vec4 MaterialAmbientColor, MaterialDiffuseColor, MaterialSpecularColor;

        MaterialAmbientColor  = vec4(vertexcolor.rgb * LightAmbient, vertexcolor.a);
        MaterialDiffuseColor  = vec4(vertexcolor.rgb * LightDiffuse, vertexcolor.a);
        MaterialSpecularColor = vec4(1.0, 1.0, 1.0, 1.0);

        // Vector that goes from the vertex to the eye, in view space.
        vec3 EyeDirection_viewSpace = EyePosition_viewSpace - Position_viewSpace;

        // Vector that goes from the vertex to the light, in view space.
        vec3 LightDirection_viewSpace = LightPosition_viewSpace - Position_viewSpace;

        // Distance to the light
        float distance = length(LightPosition_viewSpace - Position_viewSpace);

        // Normal of the computed fragment, in viewSpace
        vec3 N = normalize(Normal_viewSpace);

        // Direction from the fragment to the light
        vec3 L = normalize(LightDirection_viewSpace);

        // Cosine of the angle between the normal and the light direction, clamped above 0
        float cosTheta = 1.0;
        if(TwoSided==0) cosTheta = clamp(dot(N,L), 0.0, 1.0);
        else            cosTheta = abs(dot(N,L)); // reflection on both sides of the surface

        // Direction from the vertex to the eye, in view space.
        vec3 E = normalize(EyeDirection_viewSpace);

        // Direction in which the triangle reflects the light
        vec3 R = reflect(-L,N);

        // Cosine of the angle between the Eye vector and the Reflect vector,
        float cosAlpha = clamp(dot(E,R), 0.0, 1.0);

        float attenuation_factor = clamp(1.0/(Kc + Kl*distance + Kq*distance*distance), 0.00001, 1.0);

        fragColor =
                  MaterialAmbientColor  * LightColor +
                 (MaterialDiffuseColor  * LightDiffuse  * cosTheta)                         * LightColor * attenuation_factor
                +(MaterialSpecularColor * LightSpecular * pow(cosAlpha, MaterialShininess)) * LightColor * attenuation_factor;
    }
    else
    {
        fragColor  = vertexcolor;
    }
}
















