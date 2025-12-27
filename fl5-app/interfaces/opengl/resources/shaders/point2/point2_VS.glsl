#version 330


uniform mat4 pvmMatrix;
uniform mat4 vmMatrix;
in vec4 vertexPosition_modelSpace;
in vec4 vertexColor;

uniform float pointsize;

out vec4 pointcolor;
out vec3 Position_viewSpace;

void main(void)
{
    gl_Position =  pvmMatrix * vec4(vertexPosition_modelSpace.xyz, 1.0f);
    vec4 vsPos = vmMatrix * vec4(vertexPosition_modelSpace.xyz, 1.0f);
    Position_viewSpace = vsPos.xyz;

    gl_PointSize = pointsize;
    pointcolor = vertexColor;
}
