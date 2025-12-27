#version 330 core

//https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping

in vec4 vertexPosition_modelSpace;

uniform mat4 LightViewMatrix; // projection * view;

void main()
{
    gl_Position = LightViewMatrix * vertexPosition_modelSpace;
}
