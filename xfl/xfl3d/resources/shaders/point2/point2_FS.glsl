#version 330

uniform int HasUniColor;
uniform vec4 UniformColor;

uniform float clipPlane0; // defined in view-space

in vec3 Position_viewSpace;
in vec4 pointcolor;

layout(location=0) out vec4 fragColor;


void main(void)
{
    if (Position_viewSpace.z > clipPlane0)
    {
        discard;
        return;
    }

    vec2 coord = gl_PointCoord - vec2(0.5);  //from [0,1] to [-0.5,0.5]
    if(length(coord) > 0.5)                  //outside of circle radius?
    {
        discard;
        return;
    }

    vec4 clr;
    if(HasUniColor==1)
        clr = UniformColor;
    else
        clr = pointcolor; // from the VS

    fragColor = vec4(clr.xyz, 1.0-length(coord)*2.0);
}
