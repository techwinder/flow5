/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois 
    All rights reserved.

*****************************************************************************/

#version 330
// The surface fragment shader

in vec3 Position_viewSpace;
in vec3 Normal_viewSpace;
in vec2 UV;
in vec4 FragPosLightSpace;
in vec4 VSColor;

uniform int HasUniColor = 0; // otherwise the attribute color will be used
uniform vec4 UniformColor;
uniform int HasTexture = 0;
uniform int HasShadow = 0;
uniform int TwoSided;
uniform int LightOn;
uniform vec3 LightPosition_viewSpace;
uniform vec3 EyePosition_viewSpace;
uniform sampler2D TheSampler;

uniform vec4 LightColor;
uniform float LightAmbient, LightDiffuse, LightSpecular;
uniform float Kc, Kl, Kq;
uniform float MaterialShininess;
uniform float clipPlane0; // defined in view-space

layout(location=0) out vec4 fragColor;


float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(TheSampler, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // apply bias to avoid shadow acne
    vec3 LightDirection_viewSpace = LightPosition_viewSpace - Position_viewSpace;
    float bias = max(0.00005 * (1.0 - dot(Normal_viewSpace, LightDirection_viewSpace)), 0.0000);
//    float bias = 0.00005;
    // check whether current frag pos is in shadow
//    float shadow = currentDepth + bias > closestDepth  ? 1.0 : 0.0;

    //percentage-closer filtering,
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(TheSampler, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(TheSampler, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // no shadow if outside the light's perspective projection
    if(closestDepth<0 || closestDepth>1) shadow = 0.0;
    if(currentDepth<0 || currentDepth>1) shadow = 0.0;
    if(projCoords.z > 1.0)               shadow = 0.0;
    return shadow;
}


void main()
{
    if (Position_viewSpace.z > clipPlane0)
    {
        discard;
        return;
    }

    vec4 fragcolor;
    if(HasUniColor==1) fragcolor = UniformColor;
    else               fragcolor = VSColor; // incoming from the Vertex Shader


    if(LightOn==1)
    {
        // Material properties
        vec4 MaterialAmbientColor, MaterialDiffuseColor, MaterialSpecularColor;
        if(HasTexture==1)
        {
            MaterialAmbientColor  = vec4(texture(TheSampler, UV).rgb*LightAmbient, 1.0);
            MaterialDiffuseColor  = vec4(texture(TheSampler, UV).rgb*LightDiffuse, 1.0);
//            MaterialAmbientColor  = texture(TheSampler, UV)*LightAmbient;
//            MaterialDiffuseColor  = texture(TheSampler, UV)*LightDiffuse;
            MaterialSpecularColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
        else
        {
            MaterialAmbientColor  = vec4(fragcolor.rgb * LightAmbient, fragcolor.a);
            MaterialDiffuseColor  = vec4(fragcolor.rgb * LightDiffuse, fragcolor.a);
            MaterialSpecularColor = vec4(1.0, 1.0, 1.0, 1.0);
        }

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

        if(HasShadow==0)
        {
            fragColor =
                      MaterialAmbientColor  * LightColor +
                     (MaterialDiffuseColor  * LightDiffuse  * cosTheta)                         * LightColor * attenuation_factor
                    +(MaterialSpecularColor * LightSpecular * pow(cosAlpha, MaterialShininess)) * LightColor * attenuation_factor;
        }
        else
        {
            float shadow = ShadowCalculation(FragPosLightSpace);
    //        float shadow = 0.0;
            fragColor =
                      MaterialAmbientColor  * LightColor +
                     (MaterialDiffuseColor  * LightDiffuse  * cosTheta)                         * LightColor * attenuation_factor * (1.0-shadow)
                    +(MaterialSpecularColor * LightSpecular * pow(cosAlpha, MaterialShininess)) * LightColor * attenuation_factor * (1.0-shadow);
        }
    }
    else
    {
        if(HasTexture==0)
            fragColor  = fragcolor;
        else
        {
            fragColor  = vec4(texture(TheSampler, UV).rgb, 1.0);
//            fragColor  = texture(TheSampler, UV);
        }
    }
}
















