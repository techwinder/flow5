#version 330

layout(points) in;


// max_vertices is for the whole shader, not per primitive
// N triangles x 3 vertices

layout(triangle_strip, max_vertices = 128) out;

uniform mat4 pvmMatrix;
uniform mat4 vmMatrix;
uniform vec2 Viewport;
uniform float Thickness = 1.0;
uniform int Shape = 0;



in float pointstate[];
in vec4 VtxColor[];

out vec3 Position_viewSpace;  // passed to the fragment shader for plane clipping
out vec3 Normal_viewSpace;
out vec4 VSColor;


void emitPolygon(vec4 pos, float thck)
{
    int NTRIANGLES  = 17;

    float density = 1.0;

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
        Position_viewSpace = vsPos.xyz / vsPos.w;          // depth position for clip plane
        density = 1.0;
        Normal_viewSpace = vec3(0,0,1);
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        gl_Position = center + vec4(cost_j1/Viewport.x, sint_j1/Viewport.y,0,0);   // convert back 2d to 3d
        Position_viewSpace = vsPos.xyz / vsPos.w;
        density = 0.0;
        Normal_viewSpace = vec3(0,0,1);
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        gl_Position = center + vec4(cost_j/Viewport.x, sint_j/Viewport.y,0,0);   // convert back 2d to 3d
        Position_viewSpace = vsPos.xyz / vsPos.w;
        density = 0.0;
        Normal_viewSpace = vec3(0,0,1);
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();


        EndPrimitive();
    }
}



// emiiting 2-triangle triangle_strips to enable flat face normals
void emitCube(vec4 pos, float halfside)
{
    float density = 1.0;

    // make a cube centered on the vertex using two triangle_strips
    vec4 vertex, pvmpos, vsPos;

    vec4 Node, Normal; // model space

    // bottom face
    // Start triangle strip primitive
    {
        Node = vec4(-1, -1,-1, 0);
        Normal = vec4(0,0,-1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();
        // After calling this function, all output variables contain undefined values.
        // So you will need to write to them all again before emitting the next vertex

        Node = vec4( 1,-1,-1,0);
        Normal = vec4(0,0,-1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(-1,1,-1,0);
        Normal = vec4(0,0,-1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(1,1,-1,0);
        Normal = vec4(0,0,-1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();
    }

    EndPrimitive();


    // y-front face
    // Start triangle strip primitive
    {
        Normal = vec4(0,1,0,0);

        Node = vec4(-1,1,-1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4( 1,1,-1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(-1,1,1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4( 1,1,1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();
    }
    EndPrimitive();

    // top face
    // Start triangle strip primitive
    {
        Normal = vec4(0,0,1,0);

        Node = vec4(-1,-1,1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4( 1,-1,1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(-1,1,1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(1,1,1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();
    }
    EndPrimitive();


    // y-back
    // Start triangle strip primitive
    {
        Normal = vec4(0,-1,0,0);

        Node = vec4(-1,-1,-1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4( 1,-1,-1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(-1,-1, 1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(1,-1,1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();
    }
    EndPrimitive();


    // x-front
    // Start triangle strip primitive
    {
        Normal = vec4(1,0,0,0);

        Node = vec4(1,-1,-1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(1,1,-1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(1,-1,1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(1,1,1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();
    }
    EndPrimitive();


    // x-back
    // Start triangle strip primitive
    {
        Normal = vec4(-1,0,0,0);

        Node = vec4(-1,1,-1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(-1,-1,-1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(-1,1,1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(-1,-1,1,0);
        vertex = pos + halfside*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();
    }

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

    float density = 1.0;

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

    vec4 Node, Normal; // model space

    //make the top five triangles from the North pole to the northern hemisphere latitude


    for(int i=0; i<5; i++)
    {
        int ipole=10;
        int i1 = i;
        int i2 = (i+1)%5;

        // emit North pole
        Node = vec4(vtx[3*ipole], vtx[3*ipole+1], vtx[3*ipole+2], 0);
        Normal = Node;
        vertex = pos + side*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(vtx[3*i1], vtx[3*i1+1], vtx[3*i1+2], 0);
        Normal = Node;
        vertex = pos + side*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(vtx[3*i2], vtx[3*i2+1], vtx[3*i2+2], 0);
        Normal = Node;
        vertex = pos + side*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
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

        Node = vec4(vtx[3*ipole], vtx[3*ipole+1], vtx[3*ipole+2], 0);
        Normal = Node;
        vertex = pos + side*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(vtx[3*i2], vtx[3*i2+1], vtx[3*i2+2], 0);
        Normal = Node;
        vertex = pos + side*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        Node = vec4(vtx[3*i1], vtx[3*i1+1], vtx[3*i1+2], 0);
        Normal = Node;
        vertex = pos + side*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();

        EndPrimitive();
    }

    // make the equatorial belt
    for(int i=0; i<=5; i++)
    {
        int i1 = i%5;
        int i2 = 5+i%5;

        Node = vec4(vtx[3*i1], vtx[3*i1+1], vtx[3*i1+2], 0);
        Normal = Node;
        vertex = pos + side*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
        EmitVertex();


        Node = vec4(vtx[3*i2], vtx[3*i2+1], vtx[3*i2+2], 0);
        Normal = Node;
        vertex = pos + side*Node;   // convert back 2d to 3d
        vsPos = vmMatrix * vertex; // position of vertex in viewspace
        pvmpos = pvmMatrix*vertex;
        gl_Position = vec4(pvmpos.x/pvmpos.w, pvmpos.y/pvmpos.w, pvmpos.z/pvmpos.w, 1.0);
        Position_viewSpace = vsPos.xyz / vsPos.w;
        Normal_viewSpace = vec3(vmMatrix * Normal);
        density = 1.0;
        VSColor = vec4(VtxColor[0].xyz, density);
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
            emitPolygon(pos, thck);
            break;
        case 1:
            emitIcosahedron(pos, thck);
            break;
        case 2:
            emitCube(pos, thck);
            break;
    }
}





