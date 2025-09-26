#version 330

in vec4 vertexPosition_modelSpace;
in vec4 vertexColor;

out vec4 VtxColor;

void main(void)
{
    VtxColor = vertexColor;
    gl_Position =  vertexPosition_modelSpace;// pass through, the processing is done in the geom shader
}
