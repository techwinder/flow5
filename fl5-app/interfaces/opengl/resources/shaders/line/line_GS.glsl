#version 330

layout(lines) in;


// max_vertices is for the whole shader, not per primitive
// 2 end points x (2 lines + 6x3 triangle vertices)
layout(triangle_strip, max_vertices = 40) out;

uniform mat4 pvmMatrix;
uniform mat4 vmMatrix;
uniform float Thickness;
uniform vec2 Viewport;

in vec4 VtxColor[];


out vec3 vPosition;  // passed to the fragment shader for plane clipping
out float texCoord;  // passed to the fragment shader for stipple pattern
out vec4 GeomColor;

void main(void)
{
    float thck = Thickness/1500.0;

    //density is independent of line orientation
//    float max_u_texture = length(pvmMatrix*(gl_in[1].gl_Position - gl_in[0].gl_Position))/max(1,Thickness);

    // convert the two endpoints to screen space
    vec4 pvm[2] = vec4[](pvmMatrix * gl_in[0].gl_Position, pvmMatrix * gl_in[1].gl_Position);

    vec2 v[2];
    v[0] = vec2(pvm[0].xy/pvm[0].w) * Viewport;
    v[1] = vec2(pvm[1].xy/pvm[1].w) * Viewport;

    // to achieve uniform pattern density whatever the line orientation and the view orientation
    // the upper texture coordinate is made proportional to the projected line's length
    float texture[] = float[](0, length(v[1] - v[0]));


    vec2 u= normalize(v[1]-v[0]);
    vec2 n= vec2(-u.y,u.x);

    for(int i=0; i<2; i++)
    {
        vec4 vsPos = vmMatrix * gl_in[i].gl_Position;
        // Line Start
        gl_Position = vec4((v[i]-n*thck)/Viewport, pvm[i].z/pvm[i].w, 1.0);
        vPosition = vsPos.xyz / vsPos.w;
        texCoord = texture[i];
        GeomColor = VtxColor[i];
        EmitVertex();

        gl_Position = vec4((v[i]+n*thck)/Viewport, pvm[i].z/pvm[i].w, 1.0);
        vPosition = vsPos.xyz / vsPos.w;
        texCoord = texture[i];
        GeomColor = VtxColor[i];
        EmitVertex();

    }

    EndPrimitive();// done


    // make two triangle fans at each end point
    // goal is double:
    //   - if endpoints are at the same position, this will display an hexagon
    //   - avoid gaps at endpoints when connecting lines
    // glsl does not accept triangle fans, so make 6 triangles instead

/*    for(int i=0; i<0; i++)
    {
        vec4 vsPos = vmMatrix * gl_in[i].gl_Position;
        for(int j=0; j<6; j++)
        {
            float theta_j = float(j)*3.1416/3.0;
            float cost_j = thck*cos(theta_j);
            float sint_j = thck*sin(theta_j);
            float theta_j1 = float(j+1)*3.1416/3.0;
            float cost_j1 = thck*cos(theta_j1);
            float sint_j1 = thck*sin(theta_j1);

            gl_Position = pvm[i];   // convert back 2d to 3d
            vPosition = vsPos.xyz / vsPos.w;          // depth position for clip plane
            texCoord = texture[i];                           // set start texture coordinate
            GeomColor = VtxColor[i];
            EmitVertex();

            gl_Position = pvm[i] + vec4(cost_j/Viewport.x,sint_j/Viewport.y,0,0);   // convert back 2d to 3d
            vPosition = vsPos.xyz / vsPos.w;
            texCoord = texture[i];
            GeomColor = VtxColor[i];
            EmitVertex();

            gl_Position = pvm[i] + vec4(cost_j1/Viewport.x,sint_j1/Viewport.y,0,0);   // convert back 2d to 3d
            vPosition = vsPos.xyz / vsPos.w;
            texCoord = texture[i];
            GeomColor = VtxColor[i];
            EmitVertex();
        }
        EndPrimitive();
    }*/
}





