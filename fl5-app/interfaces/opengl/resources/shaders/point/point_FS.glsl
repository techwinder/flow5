#version 330

// point fragment shader

uniform float clipPlane0; // defined in view-space

in vec3 vPosition;
in vec3 Normal_viewSpace;
in float density;
in float state;

uniform int TwoSided;
uniform int LightOn;
uniform vec3 LightPosition_viewSpace;
uniform vec3 EyePosition_viewSpace;
uniform vec4 LightColor;
uniform float LightAmbient, LightDiffuse, LightSpecular;
uniform float Kc, Kl, Kq;
uniform float MaterialShininess;

uniform vec4 Color = vec4(1.0, 1.0, 1.0, 1.0); // used if state is invalid


layout(location=0) out vec4 fragColor;

float glGetRed(float tau)
{
    if     (tau>2.0f/3.0f) return 1.0f;
    else if(tau>1.0f/3.0f) return (3.0f*(tau-1.0f/3.0f));
    else                   return 0.0f;
}

float glGetGreen(float tau)
{
    if     (tau<0.0f || tau>1.0f) return 0.0f;
    else if(tau<1.0f/4.0f)        return (4.0f*tau);
    else if(tau>3.0f/4.0f)        return (1.0f-4.0f*(tau-3.0f/4.0f));
    else                          return 1.0;
}

float glGetBlue(float tau)
{
    if(tau>2.0f/3.0f)      return 0.0f;
    else if(tau>1.0f/3.0f) return (1.0f-3.0f*(tau-1.0f/3.0f));
    else                   return 1.0f;
}

void main(void)
{
    if (vPosition.z > clipPlane0) {
        discard;
        return;
    }
    // note: density incompatible with depth testing
    vec4 vertexcolor;
    if(state<0 || state>1)
    {
        vertexcolor = Color; // use the uniform
        vertexcolor.a = density;
    }
    else
        vertexcolor = vec4(glGetRed(state), glGetGreen(state), glGetBlue(state), density);

    if(LightOn==1)
    {
        // Material properties
        vec4 MaterialAmbientColor  = vec4(vertexcolor.rgb * LightAmbient, vertexcolor.a);
        vec4 MaterialDiffuseColor  = vec4(vertexcolor.rgb * LightDiffuse, vertexcolor.a);
        vec4 MaterialSpecularColor = vec4(1.0, 1.0, 1.0, 1.0);

        // Vector that goes from the vertex to the eye, in view space.
        vec3 EyeDirection_viewSpace = EyePosition_viewSpace - vPosition;

        // Vector that goes from the vertex to the light, in view space.
        vec3 LightDirection_viewSpace = LightPosition_viewSpace - vPosition;

        // Distance to the light
        float distance = length(LightPosition_viewSpace - vPosition);

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
