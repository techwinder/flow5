#version 330

//the point vertex shader
in vec4 vertexPosition_modelSpace;
in vec4 vertexColor;

out vec4 VtxColor;

void main(void)
{
    gl_Position =  vertexPosition_modelSpace;
    VtxColor = vertexColor;

    // restore w component
    gl_Position.w = 1.0;
}
