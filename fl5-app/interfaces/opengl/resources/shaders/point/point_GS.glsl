#version 330

layout(points) in;


// max_vertices is for the whole shader, not per primitive
// N triangles x 3 vertices
#define NTRIANGLES 20
layout(triangle_strip, max_vertices = 128) out;

uniform mat4 pvmMatrix;
uniform mat4 vmMatrix;
uniform vec2 Viewport;
uniform float Thickness;
uniform int Shape;



in float pointstate[];

out vec3 vPosition;  // passed to the fragment shader for plane clipping
out vec3 Normal_viewSpace;
out float density; // passed to the fragment shader for alpha clr
out float state;



void emitPentagon(vec4 pos, float thck)
{
    vec4 vsPos = vmMatrix * pos;
    vec4 center = pvmMatrix * pos;
    Normal_viewSpace = vec3(0,0,1);
    // make a triangle fan at the input point
    // glsl does not accept triangle fans, so make triangles instead
    for(int j=0; j<NTRIANGLES; j++)
    {
        float theta_j = float(j)*2.0*3.141592654/float(NTRIANGLES);
        float cost_j = thck*cos(theta_j);
        float sint_j = thck*sin(theta_j);
        float theta_j1 = float(j+1)*2.0*3.141592654/float(NTRIANGLES);
        float cost_j1 = thck*cos(theta_j1);
        float sint_j1 = thck*sin(theta_j1);

        gl_Position = center;   // convert back 2d to 3d
        vPosition = vsPos.xyz / vsPos.w;          // depth position for clip plane
        state  = pointstate[0];
        density = 1.0;
        Normal_viewSpace = vec3(0,0,1);
        EmitVertex();

        gl_Position = center + vec4(cost_j1/Viewport.x, sint_j1/Viewport.y,0,0);   // convert back 2d to 3d
        vPosition = vsPos.xyz / vsPos.w;
        state  = pointstate[0];
        density = 0.0;
        Normal_viewSpace = vec3(0,0,1);
        EmitVertex();

        gl_Position = center + vec4(cost_j/Viewport.x, sint_j/Viewport.y,0,0);   // convert back 2d to 3d
        vPosition = vsPos.xyz / vsPos.w;
        state  = pointstate[0];
        density = 0.0;
        Normal_viewSpace = vec3(0,0,1);
        EmitVertex();


        EndPrimitive();
    }
}


void emitCube(vec4 pos, float halfside)
{
    // make a cube centered on the vertex using two triangle_strips
    vec4 vertex, pvmpos, vsPos;

    vec4 vtxNormal; // model space
    // first strip
    // bottom, y-front, top, y-back
    // bottom face

    vtxNormal = vec4(+1, -1,-1, 0);
//    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    vtxNormal = vec4(-1,-1,-1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();
    // After calling this function, all output variables contain undefined values.
    // So you will need to write to them all again before emitting the next vertex

    vtxNormal = vec4(1,1,-1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    vtxNormal = vec4(-1,1,-1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();


    // y-front face
    vtxNormal = vec4(1,1,1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    vtxNormal = vec4(-1,1,1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    // top face
    vtxNormal = vec4(1,-1,1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    vtxNormal = vec4(-1,-1,1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    // y-back
    vtxNormal = vec4(1,-1,-1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    vtxNormal = vec4(-1,-1,-1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    EndPrimitive();


    // x-front
    vtxNormal = vec4(1,-1,-1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    vtxNormal = vec4(1,1,-1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    vtxNormal = vec4(1,-1,1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    vtxNormal = vec4(1,1,1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    EndPrimitive();


    // x-back
    vtxNormal = vec4(-1,1,-1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    vtxNormal = vec4(-1,-1,-1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    vtxNormal = vec4(-1,1,1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();

    vtxNormal = vec4(-1,-1,1,0);
    vertex = pos + halfside*vtxNormal;   // convert back 2d to 3d
    vsPos = vmMatrix * vertex; // position of vertex in viewspace
    pvmpos = pvmMatrix*vertex;
    gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
    vPosition = vsPos.xyz / vsPos.w;
    Normal_viewSpace = vec3(vmMatrix * vtxNormal);
    state  = pointstate[0];
    density = 1.0;
    EmitVertex();


    EndPrimitive();
}


void emitIcosahedron(vec4 pos, float side)
{
    float PI = 3.141592654;
    float radius = 1.0;
    // make vertices
    float vtx[36], normal[36];
    //North pole
    vtx[30] = 0;
    vtx[31] = 0;
    vtx[32] = radius;

    //South pole
    vtx[33] = 0;
    vtx[34] = 0;
    vtx[35] = -radius;

    float x=0,y=0,z=0;
    float atn= atan(0.5);
    float di=0.0;
    for(int i=0; i<5; i++)
    {
        di = float(i);
        x = radius * cos(atn)*cos(72.0*di*PI/180.0);
        y = radius * cos(atn)*sin(72.0*di*PI/180.0);
        z = radius * sin(atn);
        vtx[3*i+0]=x;
        vtx[3*i+1]=y;
        vtx[3*i+2]=z;
        normal[3*i+0]=x;
        normal[3*i+1]=y;
        normal[3*i+2]=z;

        x =  radius * cos(atn)*cos((36+72.0*di)*PI/180.0);
        y =  radius * cos(atn)*sin((36+72.0*di)*PI/180.0);
        z = -radius * sin(atn);
        vtx[3*(i+5)+0]=x;
        vtx[3*(i+5)+1]=y;
        vtx[3*(i+5)+2]=z;
        normal[3*(i+5)+0]=x;
        normal[3*(i+5)+1]=y;
        normal[3*(i+5)+2]=z;
    }


    // 20 triangles
    // x3vertices/triangle
    // 3 coordinates/vertex
    vec4 vertex, pvmpos, vsPos;

    vec4 vtxNormal; // model space

    //make the top five triangles from the North pole to the northern hemisphere latitude


    for(int i=0; i<5; i++)
    {
        int ipole=10;
        int i1 = i;
        int i2 = (i+1)%5;

        // emit North pole
        vtxNormal = vec4(vtx[3*ipole], vtx[3*ipole+1], vtx[3*ipole+2], 0);
        vertex = pos + side*vtxNormal;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        vPosition = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * vtxNormal);
        state  = pointstate[0];
        density = 1.0;
        EmitVertex();

        vtxNormal = vec4(vtx[3*i1], vtx[3*i1+1], vtx[3*i1+2], 0);
        vertex = pos + side*vtxNormal;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        vPosition = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * vtxNormal);
        state  = pointstate[0];
        density = 1.0;
        EmitVertex();

        vtxNormal = vec4(vtx[3*i2], vtx[3*i2+1], vtx[3*i2+2], 0);
        vertex = pos + side*vtxNormal;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        vPosition = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * vtxNormal);
        state  = pointstate[0];
        density = 1.0;
        EmitVertex();

        EndPrimitive();
    }


    // make the bottom five triangles from the South pole to the northern hemisphere latitude
    // emit South pole
    for(int i=4; i>=0; i--)
    {
        int ipole=11;
        int i1 = 5+i;
        int i2 = 5+(i+1)%5;

        vtxNormal = vec4(vtx[3*ipole], vtx[3*ipole+1], vtx[3*ipole+2], 0);
        vertex = pos + side*vtxNormal;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        vPosition = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * vtxNormal);
        state  = pointstate[0];
        density = 1.0;
        EmitVertex();

        vtxNormal = vec4(vtx[3*i2], vtx[3*i2+1], vtx[3*i2+2], 0);
        vertex = pos + side*vtxNormal;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        vPosition = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * vtxNormal);
        state  = pointstate[0];
        density = 1.0;
        EmitVertex();

        vtxNormal = vec4(vtx[3*i1], vtx[3*i1+1], vtx[3*i1+2], 0);
        vertex = pos + side*vtxNormal;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        vPosition = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * vtxNormal);
        state  = pointstate[0];
        density = 1.0;
        EmitVertex();

        EndPrimitive();
    }

    // make the equatorial belt
    for(int i=0; i<=5; i++)
    {
        int i1 = i%5;
        int i2 = 5+i%5;

        vtxNormal = vec4(vtx[3*i1], vtx[3*i1+1], vtx[3*i1+2], 0);
        vertex = pos + side*vtxNormal;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        vPosition = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * vtxNormal);
        state  = pointstate[0];
        density = 1.0;
        EmitVertex();


        vtxNormal = vec4(vtx[3*i2], vtx[3*i2+1], vtx[3*i2+2], 0);
        vertex = pos + side*vtxNormal;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        vPosition = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * vtxNormal);
        state  = pointstate[0];
        density = 1.0;
        EmitVertex();
    }
    EndPrimitive();
}



void main(void)
{
    vec4 pos = gl_in[0].gl_Position;

    float thck = float(Thickness)/100.0/Viewport.x; // so that 1 pt is 1% viewport width
    switch(Shape)
    {
        default:
        case 0:
            emitPentagon(pos, thck);
            break;
        case 1:
            emitIcosahedron(pos, thck);
            break;
        case 2:
            emitCube(pos, thck);
            break;
    }
}





