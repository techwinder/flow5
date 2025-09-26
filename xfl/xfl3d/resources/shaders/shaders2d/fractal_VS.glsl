#version 330

uniform vec2  ViewTrans;
uniform float ViewScale;
uniform float ViewRatio;

layout(location = 0) in vec2 VertexPosition;

out vec2 pos;

void main(void)
{
    pos.x = (VertexPosition.x/ViewScale           + ViewTrans.x);
    pos.y = (VertexPosition.y/ViewScale/ViewRatio + ViewTrans.y);
    gl_Position = vec4(VertexPosition.x, VertexPosition.y, 0.0f, 1.0f);
}
