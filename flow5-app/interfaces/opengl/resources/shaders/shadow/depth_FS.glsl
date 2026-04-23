#version 330 core


//https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
void main()
{
//    Since we have no color buffer and disabled the draw and read buffers,
//    the resulting fragments do not require any processing so we can simply use an empty fragment shader:

//     gl_FragDepth = gl_FragCoord.z;
}
//This empty fragment shader does no processing whatsoever, and at the end of its run the depth buffer is updated.
//We could explicitly set the depth by uncommenting its one line, but this is effectively what happens behind the scene anyways.
