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

float fmaf(float x, float y, float z)
{
    return x*y+z;
}

/* compute inverse error functions with maximum error of 2.35793 ulp */
float erf_inv(float a)
{
    float p = 0;
    float r = 0;
    float t = fmaf (a, 0.0f - a, 1.0f); //Computes (x * y) + z as if to infinite precision and rounded only once to fit the result type.
    t = log(t);
    if (abs(t) > 6.125f)
    { // maximum ulp error = 2.35793
        p =              3.03697567e-10f; //  0x1.4deb44p-32
        p = fmaf (p, t,  2.93243101e-8f); //  0x1.f7c9aep-26
        p = fmaf (p, t,  1.22150334e-6f); //  0x1.47e512p-20
        p = fmaf (p, t,  2.84108955e-5f); //  0x1.dca7dep-16
        p = fmaf (p, t,  3.93552968e-4f); //  0x1.9cab92p-12
        p = fmaf (p, t,  3.02698812e-3f); //  0x1.8cc0dep-9
        p = fmaf (p, t,  4.83185798e-3f); //  0x1.3ca920p-8
        p = fmaf (p, t, -2.64646143e-1f); // -0x1.0eff66p-2
        p = fmaf (p, t,  8.40016484e-1f); //  0x1.ae16a4p-1
    }
    else
    { // maximum ulp error = 2.35456
        p =              5.43877832e-9f;  //  0x1.75c000p-28
        p = fmaf (p, t,  1.43286059e-7f); //  0x1.33b458p-23
        p = fmaf (p, t,  1.22775396e-6f); //  0x1.49929cp-20
        p = fmaf (p, t,  1.12962631e-7f); //  0x1.e52bbap-24
        p = fmaf (p, t, -5.61531961e-5f); // -0x1.d70c12p-15
        p = fmaf (p, t, -1.47697705e-4f); // -0x1.35be9ap-13
        p = fmaf (p, t,  2.31468701e-3f); //  0x1.2f6402p-9
        p = fmaf (p, t,  1.15392562e-2f); //  0x1.7a1e4cp-7
        p = fmaf (p, t, -2.32015476e-1f); // -0x1.db2aeep-3
        p = fmaf (p, t,  8.86226892e-1f); //  0x1.c5bf88p-1
    }
    r = a * p;
    return r;
}


vec4 startpos(vec4 oldpos)
{
    float mean = 0.0;
    float sigma = REFLENGTH;

    vec4 newpos;
    newpos.x = -REFLENGTH/2.0f;

    // raising the digits to get pseudo rand
    float flt = 147.51861127f;
    float randy = oldpos.y*flt-floor(oldpos.y*flt); // in [0, 1[
    float randz = oldpos.z*flt-floor(oldpos.z*flt); // in [0, 1[

    randy = randy * 2.0 - 1.0f; // in ]-1, 1[
    randy = erf_inv(randy) * sigma + mean;

    randz = randz * 2.0 - 1.0f; // in ]-1, 1[
    randz = erf_inv(randz) * sigma + mean;

//    newpos.y = (randy*2.0-1.0)*REFLENGTH;
//    newpos.z = (randz*1.0-0.5)*REFLENGTH;
    newpos.y = (randy)*REFLENGTH;
    newpos.z = (randz)*REFLENGTH/2.0;
    newpos.w = 1.0;
    return newpos;
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
        newpos = startpos(oldpos);
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






