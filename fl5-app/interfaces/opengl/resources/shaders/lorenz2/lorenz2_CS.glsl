#version 430

uniform float radius;
uniform float dt; // in ms

// Declare the custom data type that represents one point of a circle.
// This is vertex position and color respectively,
// that defines the interleaved data within a
// buffer that is Vertex|Color|Vertex|Color|
struct AttribData
{
    vec4 v;
    vec4 c;
};

// Declare an input/output buffer that stores data.
// This shader only writes data into the buffer.
// std430 is a standard packing layout which is preferred for SSBOs.
// Its binary layout is well defined.
// Bind the buffer to index 0. You must set the buffer binding
// in the range [0..3]. This is the minimum range approved by Khronos.
// Some platforms might support more indices.
layout(shared, binding = 0) buffer SSBO
{
    AttribData data[];
} outBuffer;

// Declare the group size.
#define LORENZ_GROUP_SIZE 512
layout (local_size_x = LORENZ_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;


float glGetRed(float tau)
{
    if     (tau>2.0f/3.0f) return 1.0f;
    else if(tau>1.0f/3.0f) return (3.0f*(tau-1.0f/3.0f));
    else                   return 0.0f;
}

float glGetGreen(float tau)
{
    if     (tau<0.0f || tau>1.0f) return 0.0f;
    else if(tau<1.0f/4.0f)        return (4.0f*tau);
    else if(tau>3.0f/4.0f)        return (1.0f-4.0f*(tau-3.0f/4.0f));
    else                          return 1.0;
}

float glGetBlue(float tau)
{
    if(tau>2.0f/3.0f)      return 0.0f;
    else if(tau>1.0f/3.0f) return (1.0f-3.0f*(tau-1.0f/3.0f));
    else                   return 1.0f;
}

#define sigma 10.0
#define beta 2.6667
#define rho 28.0

float f(float x, float y, float z)  {return sigma*(y-x);}
float g(float x, float y, float z)  {return x*(rho-z)-y;}
float h(float x, float y, float z)  {return x*y-beta*z;}


vec4 moveIt(vec4 pt)
{
    vec4 ptout = pt;

    //predictor
    float k1 = f(pt.x,           pt.y,             pt.z);
    float l1 = g(pt.x,           pt.y,             pt.z);
    float m1 = h(pt.x,           pt.y,             pt.z);

    float k2 = f(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));
    float l2 = g(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));
    float m2 = h(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));

    float k3 = f(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));
    float l3 = g(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));
    float m3 = h(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));

    float k4 = f(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);
    float l4 = g(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);
    float m4 = h(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);

    //corrector
    ptout.x += dt*(k1 +2*k2 +2*k3 +k4)/6;
    ptout.y += dt*(l1 +2*l2 +2*l3 +l4)/6;
    ptout.z += dt*(m1 +2*m2 +2*m3 +m4)/6;

    return ptout;
}


void main()
{
        // Read the current global position for this thread
        uint storePos = gl_GlobalInvocationID.x;

        // Calculate the global number of threads (size) for this work dispatch.
//        uint gSize = gl_WorkGroupSize.x * gl_NumWorkGroups.x;


        // Calculate the vertex position
        vec4 oldpos = outBuffer.data[storePos].v;
        vec4 newpos = moveIt(outBuffer.data[storePos].v);
        outBuffer.data[storePos].v = newpos;


        float dx = f(newpos.x, newpos.y, newpos.z);
        float dy = g(newpos.x, newpos.y, newpos.z);
        float dz = h(newpos.x, newpos.y, newpos.z);
        float step = sqrt(dx*dx+dy*dy+dz*dz)/300.0;
        outBuffer.data[storePos].c = vec4(glGetRed(step), glGetGreen(step), glGetBlue(step), 1.0);
}





