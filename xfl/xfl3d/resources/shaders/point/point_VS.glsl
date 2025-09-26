#version 330

//the point vertex shader
in vec4 vertexPosition_modelSpace;
in float PointState;

out float pointstate;

void main(void)
{
    gl_Position =  vertexPosition_modelSpace;
    pointstate = PointState;
}
