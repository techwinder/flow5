#version 330

//the point vertex shader
in vec4 vertexPosition_modelSpace;
in vec4 vertexColor;
//in float PointState;

out float pointstate;
out vec4 VtxColor;

void main(void)
{
    gl_Position =  vertexPosition_modelSpace;
    VtxColor = vertexColor;

    // use .w component for state
    pointstate = vertexPosition_modelSpace.w;

    // restore w component
    gl_Position.w = 1.0;
}
