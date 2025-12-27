#version 430

#define PI 3.141592654f
#define GROUP_SIZE 64
#define REFLENGTH 1.0f

// todo: sync with value in gl3dView
#define TRACESEGS 32

uniform int randseed;
uniform int nvortices;
uniform float gamma;
uniform float vinf;
uniform float dt;


// Watch out for padding constraints of layouts 430/140 specifically for vec3:
// https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)
// using vec4 everywhere
struct Boid
{
    vec4 pos;
    vec4 vel;
    vec4 clr;
};


struct Trace
{
    vec4 vecs[TRACESEGS*2*2]; // TRACESEGS x 2 vertices x (pos4 + clr4)
};


struct Vortex
{
    //Layout 430: The array stride (the bytes between array elements) is always rounded up to the size of a vec4
    // https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)
    vec4 A;
    vec4 B;
};


layout(std430, binding = 0) buffer SSBO_0
{
    Boid data[];
} BoidBuffer;


layout(std430, binding = 1) buffer SSBO_1
{
    Vortex data[];
}  VortexBuffer;


layout(shared, binding=2) buffer SSBO_2
{
    Trace data[];
} TraceBuffer;



layout (local_size_x = GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;

float glGetRed(float tau)
{
    if     (tau>5.0f/6.0f) return 1.0f;
    else if(tau>4.0f/6.0f) return (6.0f*(tau-4.0f/6.0f));
    else if(tau>2.0f/6.0f) return 0.0f;
    else if(tau>1.0f/6.0f) return 1.0f - (6.0f*(tau-1.0f/6.0f));
    else                   return 1.0f;
}

float glGetGreen(float tau)
{
    if      (tau<2.0f/6.0f) return 0.0f;
    else if (tau<3.0f/6.0f) return 6.0f*(tau-2.0f/6.0f);
    else if (tau<5.0f/6.0f) return 1.0f;
    else if (tau<6.0f/6.0f) return 1.0f - (6.0f*(tau-5.0f/6.0f));
    else                    return 0.0f;
}

float glGetBlue(float tau)
{
    if      (tau<0.0f)      return 0.0f;
    else if (tau<1.0f/6.0f) return 6.0f * tau;
    else if (tau<3.0f/6.0f) return 1.0f;
    else if (tau<4.0f/6.0f) return 1.0f - (6.0f*(tau-3.0f/6.0f));
    else                    return 0.0f;
}


vec4 VLMCmn(vec4 A, vec4 B, vec4 C)
{
    float CoreSize = 0.01f;
    float ftmp=0.0f, Omega=0.0f, Psi_x=0.0f, Psi_y=0.0f, Psi_z=0.0f;
    float r0_x=0.0f, r0_y=0.0f, r0_z=0.0f, r1_x=0.0f, r1_y=0.0f, r1_z=0.0f, r2_x=0.0f, r2_y=0.0f, r2_z=0.0f;
    float Far_x=0.0f, Far_y=0.0f, Far_z=0.0f, t_x=0.0f, t_y=0.0f, t_z=0.0f, h_x=0.0f, h_y=0.0f, h_z=0.0f;
    vec4 V = vec4(0,0,0,0);

    int bAll = 1;

    if(bAll==1)
    {
        r0_x = B.x - A.x;
        r0_y = B.y - A.y;
        r0_z = B.z - A.z;

        r1_x = C.x - A.x;
        r1_y = C.y - A.y;
        r1_z = C.z - A.z;

        r2_x = C.x - B.x;
        r2_y = C.y - B.y;
        r2_z = C.z - B.z;

        Psi_x = r1_y*r2_z - r1_z*r2_y;
        Psi_y =-r1_x*r2_z + r1_z*r2_x;
        Psi_z = r1_x*r2_y - r1_y*r2_x;

        ftmp = Psi_x*Psi_x + Psi_y*Psi_y + Psi_z*Psi_z;

        //get the distance of the TestPoint to the panel's side
        t_x =  r1_y*r0_z - r1_z*r0_y;
        t_y = -r1_x*r0_z + r1_z*r0_x;
        t_z =  r1_x*r0_y - r1_y*r0_x;

        if ((t_x*t_x+t_y*t_y+t_z*t_z)/(r0_x*r0_x+r0_y*r0_y+r0_z*r0_z) >CoreSize * CoreSize)
        {
            Psi_x /= ftmp;
            Psi_y /= ftmp;
            Psi_z /= ftmp;

            Omega = (r0_x*r1_x + r0_y*r1_y + r0_z*r1_z)/sqrt((r1_x*r1_x + r1_y*r1_y + r1_z*r1_z))
                    -(r0_x*r2_x + r0_y*r2_y + r0_z*r2_z)/sqrt((r2_x*r2_x + r2_y*r2_y + r2_z*r2_z));

            V.x = Psi_x * Omega/4.0/PI;
            V.y = Psi_y * Omega/4.0/PI;
            V.z = Psi_z * Omega/4.0/PI;
        }
    }
    // We create Far points to align the trailing vortices with the reference axis
    // The trailing vortex legs are not aligned with the free-stream, i.a.w. the small angle approximation
    // If this approximation is not valid, then the geometry should be tilted in the polar definition

    // calculate left contribution
    Far_x = A.x +  1.0e10;
    Far_y = A.y;
    Far_z = A.z;// + (Far_x-A.x) * tan(m_Alpha*PI/180.0);

    r0_x = A.x - Far_x;
    r0_y = A.y - Far_y;
    r0_z = A.z - Far_z;

    r1_x = C.x - A.x;
    r1_y = C.y - A.y;
    r1_z = C.z - A.z;

    r2_x = C.x - Far_x;
    r2_y = C.y - Far_y;
    r2_z = C.z - Far_z;

    Psi_x = r1_y*r2_z - r1_z*r2_y;
    Psi_y =-r1_x*r2_z + r1_z*r2_x;
    Psi_z = r1_x*r2_y - r1_y*r2_x;

    ftmp = Psi_x*Psi_x + Psi_y*Psi_y + Psi_z*Psi_z;

    t_x=1.0; t_y=0.0; t_z=0.0;

    h_x =  r1_y*t_z - r1_z*t_y;
    h_y = -r1_x*t_z + r1_z*t_x;
    h_z =  r1_x*t_y - r1_y*t_x;

    //Next add 'left' semi-infinite contribution
    //eq.6-56

    if ((h_x*h_x+h_y*h_y+h_z*h_z) > CoreSize * CoreSize)
    {
        Psi_x /= ftmp;
        Psi_y /= ftmp;
        Psi_z /= ftmp;

        Omega =  (r0_x*r1_x + r0_y*r1_y + r0_z*r1_z)/sqrt((r1_x*r1_x + r1_y*r1_y + r1_z*r1_z))
                -(r0_x*r2_x + r0_y*r2_y + r0_z*r2_z)/sqrt((r2_x*r2_x + r2_y*r2_y + r2_z*r2_z));

        V.x += Psi_x * Omega/4.0/PI;
        V.y += Psi_y * Omega/4.0/PI;
        V.z += Psi_z * Omega/4.0/PI;
    }

    // calculate right vortex contribution
    Far_x = B.x +  1.0e10;
    Far_y = B.y ;
    Far_z = B.z;// + (Far_x-B.x) * tan(m_Alpha*PI/180.0);

    r0_x = Far_x - B.x;
    r0_y = Far_y - B.y;
    r0_z = Far_z - B.z;

    r1_x = C.x - Far_x;
    r1_y = C.y - Far_y;
    r1_z = C.z - Far_z;

    r2_x = C.x - B.x;
    r2_y = C.y - B.y;
    r2_z = C.z - B.z;

    Psi_x = r1_y*r2_z - r1_z*r2_y;
    Psi_y =-r1_x*r2_z + r1_z*r2_x;
    Psi_z = r1_x*r2_y - r1_y*r2_x;

    ftmp = Psi_x*Psi_x + Psi_y*Psi_y + Psi_z*Psi_z;

    //Last add 'right' semi-infinite contribution
    h_x =  r2_y*t_z - r2_z*t_y;
    h_y = -r2_x*t_z + r2_z*t_x;
    h_z =  r2_x*t_y - r2_y*t_x;

    if ((h_x*h_x+h_y*h_y+h_z*h_z) > CoreSize * CoreSize)
    {
        Psi_x /= ftmp;
        Psi_y /= ftmp;
        Psi_z /= ftmp;

        Omega =  (r0_x*r1_x + r0_y*r1_y + r0_z*r1_z)/sqrt((r1_x*r1_x + r1_y*r1_y + r1_z*r1_z))
                -(r0_x*r2_x + r0_y*r2_y + r0_z*r2_z)/sqrt((r2_x*r2_x + r2_y*r2_y + r2_z*r2_z));

        V.x += Psi_x * Omega/4.0/PI;
        V.y += Psi_y * Omega/4.0/PI;
        V.z += Psi_z * Omega/4.0/PI;
    }
    return V;
}


void main()
{
    int inboid = int(gl_GlobalInvocationID.x);

    vec4 oldpos = BoidBuffer.data[inboid].pos;
    vec4 oldvel = BoidBuffer.data[inboid].vel;
    vec4 velocity = vec4(vinf,0,0,0);

    for(int i=0; i<nvortices; i++)
    {
        Vortex v =  VortexBuffer.data[i];
        velocity += VLMCmn(v.A, v.B, oldpos) * gamma;
    }


    bool bResetTrace = false;
    vec4 newpos = oldpos + velocity*dt;
    if(newpos.x>3.0*REFLENGTH)  bResetTrace = true;


    if(bResetTrace)
    {
        newpos.x = -REFLENGTH/2.0f;

        // raising the digits to get pseudo rand
        float flt = 147.51861127f;
        float randy = oldpos.y*flt-floor(oldpos.y*flt);
        float randz = oldpos.z*flt-floor(oldpos.z*flt);
        newpos.y = (randy*3.0-1.5)*REFLENGTH;
        newpos.z = (randz*2.0-0.5)*REFLENGTH;
    }

    BoidBuffer.data[inboid].pos = newpos;
    BoidBuffer.data[inboid].vel = velocity;

    float tau = length(velocity)/3./vinf;
    vec4 clr = vec4(glGetRed(tau), glGetGreen(tau), glGetBlue(tau), 1.0);

    BoidBuffer.data[inboid].clr = clr;

    if(!bResetTrace)
    {
        //shift 1 segment
        for(int i=TRACESEGS-1; i>0; i--)
        {
            TraceBuffer.data[inboid].vecs[4*i+0] = TraceBuffer.data[inboid].vecs[4*(i-1)+0]; //pos
            TraceBuffer.data[inboid].vecs[4*i+1] = TraceBuffer.data[inboid].vecs[4*(i-1)+1]; //clr
            TraceBuffer.data[inboid].vecs[4*i+1].w = float(TRACESEGS-1-i)/float(TRACESEGS);
            TraceBuffer.data[inboid].vecs[4*i+2] = TraceBuffer.data[inboid].vecs[4*(i-1)+2]; //pos
            TraceBuffer.data[inboid].vecs[4*i+3] = TraceBuffer.data[inboid].vecs[4*(i-1)+3]; //clr
            TraceBuffer.data[inboid].vecs[4*i+3].w = float(TRACESEGS-1-i)/float(TRACESEGS);
        }
        // update leading segment
        // endpoint is former position
        TraceBuffer.data[inboid].vecs[2] = TraceBuffer.data[inboid].vecs[0]; // pos = oldpos
        TraceBuffer.data[inboid].vecs[3] = TraceBuffer.data[inboid].vecs[1]; // clr
        //start point is new/updated position
        TraceBuffer.data[inboid].vecs[0] = newpos;
        TraceBuffer.data[inboid].vecs[1] = clr;
    }
    else
    {
        for(int i=0; i<TRACESEGS; i++)
        {
            TraceBuffer.data[inboid].vecs[4*i+0] = newpos;
            TraceBuffer.data[inboid].vecs[4*i+1] = clr;
            TraceBuffer.data[inboid].vecs[4*i+2] = newpos;
            TraceBuffer.data[inboid].vecs[4*i+3] = clr;
        }
    }
}






