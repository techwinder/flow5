#version 430

#define PI 3.141592654f
#define GROUP_SIZE 64

#define RFF 10.0f
#define INPLANEPRECISION 0.0001f

// todo: sync with value in gl3dView
#define TRACESEGS 32


uniform vec4 topleft;
uniform vec4 botright;


uniform int RK;  // 1=Euler 2=RK2 4=RK4

uniform vec4 vinf;
uniform float dt;

uniform int npanels;
uniform int nvortons;
uniform float VtnCoreSize;

uniform int HasGround; // 0=none, 1=ground effect, -1= free surface effect
uniform float GroundHeight;

uniform int HasUniColor;
uniform vec4 UniformColor;

// Watch out for padding constraints of layouts 430/140 specifically for vec3:
// https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)
// using vec4 everywhere instead
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


struct Panel3
{
    //Layout 430: The array stride (the bytes between array elements) is always rounded up to the size of a vec4
    // https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)
    vec4 floats; // area, maxsize, sigma, gamma
    vec4 S[3];
    vec4 G;
    vec4 l;
    vec4 m;
    vec4 N;
};


struct Vorton
{
    vec4 pos;
    vec4 vortex;
};


layout(shared, binding=0) buffer SSBO_0
{
    Boid data[];
} BoidBuffer;


layout(shared, binding=1) buffer SSBO_1
{
    Panel3 data[];
}  Panel3Buffer;


layout(shared, binding=2) buffer SSBO_2
{
    Vorton data[];
} VortonBuffer;


layout(shared, binding=3) buffer SSBO_3
{
    Trace data[];
} TraceBuffer;


layout (local_size_x = GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;


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

vec4 N4023Velocity(Panel3 p3, vec4 C, bool bSelf)
{
    float coreradius=0.001f;

    vec4 VelSrc = vec4(0,0,0,0);
    vec4 VelDbl = vec4(0,0,0,0);


    float PJKx = C.x - p3.G.x;
    float PJKy = C.y - p3.G.y;
    float PJKz = C.z - p3.G.z;

    float PN  = PJKx*p3.N.x + PJKy*p3.N.y + PJKz*p3.N.z;
    float pjk = sqrt(PJKx*PJKx + PJKy*PJKy + PJKz*PJKz);

    if(pjk>RFF*p3.floats.y)
    {
        // use far-field formula
        float pjk2=pjk*pjk;
        float pjk3=pjk2*pjk;
        float pjk5=pjk2*pjk3;

        VelSrc.x = PJKx * p3.floats.x/pjk3;
        VelSrc.y = PJKy * p3.floats.x/pjk3;
        VelSrc.z = PJKz * p3.floats.x/pjk3;

        float T1x = PJKx*3.0*PN - p3.N.x*pjk2;
        float T1y = PJKy*3.0*PN - p3.N.y*pjk2;
        float T1z = PJKz*3.0*PN - p3.N.z*pjk2;

        VelDbl.x  = T1x * p3.floats.x /pjk5;
        VelDbl.y  = T1y * p3.floats.x /pjk5;
        VelDbl.z  = T1z * p3.floats.x /pjk5;

        return VelSrc * p3.floats.z + VelDbl * p3.floats.w;
    }

    for (int i=0; i<3; i++)
    {
        float sx  = p3.S[(i+1)%3].x - p3.S[i%3].x;
        float sy  = p3.S[(i+1)%3].y - p3.S[i%3].y;
        float sz  = p3.S[(i+1)%3].z - p3.S[i%3].z;

        float ax  = C.x - p3.S[i%3].x;
        float ay  = C.y - p3.S[i%3].y;
        float az  = C.z - p3.S[i%3].z;

        float bx  = C.x - p3.S[(i+1)%3].x;
        float by  = C.y - p3.S[(i+1)%3].y;
        float bz  = C.z - p3.S[(i+1)%3].z;

        float A   = sqrt(ax*ax + ay*ay + az*az);
        float B   = sqrt(bx*bx + by*by + bz*bz);

        float Sk  = sqrt(sx*sx + sy*sy + sz*sz);
        float SM  = sx*p3.m.x + sy*p3.m.y + sz*p3.m.z;
        float SL  = sx*p3.l.x + sy*p3.l.y + sz*p3.l.z;
        float AM  = ax*p3.m.x + ay*p3.m.y + az*p3.m.z;
        float AL  = ax*p3.l.x + ay*p3.l.y + az*p3.l.z;
        float Al  = AM*SL - AL*SM;
        float PA  = PN*PN*SL + Al*AM;
        float PB  = PA - Al*SM;

        //get the distance of the TestPoint to the panel's side
        float hx =  ay*sz - az*sy;
        float hy = -ax*sz + az*sx;
        float hz =  ax*sy - ay*sx;

        if(Sk<coreradius)
        {
            //no contribution from this side
        }
        else if ((((hx*hx+hy*hy+hz*hz)/(sx*sx+sy*sy+sz*sz) <= coreradius*coreradius) && ax*sx+ay*sy+az*sz>=0.0 && bx*sx+by*sy+bz*sz<=0.0))
        {
            //if lying on the panel's side... no contribution
        }
        else if(A<coreradius|| B<coreradius)
        {
            //no contribution
        }
        else
        {
            // Doublet contribution

            hx =  ay*bz - az*by;
            hy = -ax*bz + az*bx;
            hz =  ax*by - ay*bx;

            float GL = ((A+B) /A/B/ (A*B + ax*bx+ay*by+az*bz));

            VelDbl.x += hx * GL;
            VelDbl.y += hy * GL;
            VelDbl.z += hz * GL;

            //Source contribution
            if(abs(A+B-Sk)>0.0) GL = 1.0/Sk * log(abs((A+B+Sk)/(A+B-Sk)));
            else                GL = 0.0;

            float RNUM = SM*PN * (B*PA-A*PB);
            float DNOM = PA*PB + PN*PN*A*B*SM*SM;

            float CJKi = 0.0;
            if(abs(PN)<INPLANEPRECISION)
            {
                // side contribution is >0 if the point is on the panel's right side
                float sign = 1.0;
                if(DNOM<0.0)
                {
                    if(PN>0.0)	CJKi =  PI * sign;
                    else        CJKi = -PI * sign;
                }
                else if(DNOM == 0.0)
                {
                    if(PN>0.0)	CJKi =  PI/2.0 * sign;
                    else        CJKi = -PI/2.0 * sign;
                }
                else
                    CJKi = 0.0;
            }
            else
            {
                CJKi = atan(RNUM, DNOM);
            }


            float T1x   = p3.l.x      * SM*GL;
            float T1y   = p3.l.y      * SM*GL;
            float T1z   = p3.l.z      * SM*GL;
            float T2x   = p3.m.x      * SL*GL;
            float T2y   = p3.m.y      * SL*GL;
            float T2z   = p3.m.z      * SL*GL;

            VelSrc.x   += p3.N.x * CJKi + T1x - T2x;
            VelSrc.y   += p3.N.y * CJKi + T1y - T2y;
            VelSrc.z   += p3.N.z * CJKi + T1z - T2z;

        }
    }

    if(abs(PN)<INPLANEPRECISION) VelSrc.z = 0.0;

/*    if(bSelf)
    {
        // The normal component of the velocity is discontinuous in the normal's direction.
        // Force the value on the outer surface to be consistent with the external BC
        VelSrc.z = p3.N.z * 2.0 * PI;
    }*/
    return VelSrc * p3.floats.z + VelDbl * p3.floats.w;
}


float mollifiedInt(float lambda)
{
    float f = (lambda*lambda+2.5) * lambda*lambda*lambda;
    float d =  sqrt((1.0+lambda*lambda)*(1.0+lambda*lambda)*(1.0+lambda*lambda)*(1.0+lambda*lambda)*(1.0+lambda*lambda));
    return f/d; // moved factor 1./4 Pi into the Kernel
}


vec4 VortonVelocity(Vorton vtn, vec4 C)
{
    if(vtn.vortex.w<0.0001) return vec4(0.0);

    //fast inline implementation
    float RpRx = C.x - vtn.pos.x;
    float RpRy = C.y - vtn.pos.y;
    float RpRz = C.z - vtn.pos.z;
    float r = sqrt(RpRx*RpRx + RpRy*RpRy + RpRz*RpRz);

    if(r<1.e-6)
    {
        return vec4(0.0);
    }

    float r3 = r*r*r;
    float Kx = -RpRx/r3;
    float Ky = -RpRy/r3;
    float Kz = -RpRz/r3;

    float f = 1.0;
    if(VtnCoreSize>0.0)
    {
        f = mollifiedInt(r/VtnCoreSize);
    }

    // Willis Eq. 21 + mollification
    vec4 V = vec4(0.0);
    V.x = ( Ky*vtn.vortex.z-Kz*vtn.vortex.y) * 1.0/4.0/PI*f;
    V.y = (-Kx*vtn.vortex.z+Kz*vtn.vortex.x) * 1.0/4.0/PI*f;
    V.z = ( Kx*vtn.vortex.y-Ky*vtn.vortex.x) * 1.0/4.0/PI*f;
    return V;
}


vec4 getVelocity(vec4 pos)
{
    vec4 velocity = vec4(0);

    vec4 CG = vec4(pos.x, pos.y, -pos.z-2.0*GroundHeight, 0.0);

    for(int i=0; i<npanels; i++)
    {
        Panel3 p3 =  Panel3Buffer.data[i];
        velocity += N4023Velocity(p3, pos, false);

        if(HasGround>0)
        {
            float coef = float(HasGround);
            vec4 VG = N4023Velocity(p3, CG, false) * coef;
            velocity.x += VG.x * coef;
            velocity.y += VG.y * coef;
            velocity.z -= VG.z * coef;
        }
    }

    /**@todo could add negating vortices to be thorough*/
    for(int i=0; i<nvortons; i++)
    {
        Vorton vtn =  VortonBuffer.data[i];
        velocity += VortonVelocity(vtn, pos);
        if(HasGround>0)
        {
            float coef = float(HasGround);
            vec4 VG = VortonVelocity(vtn, CG);
            velocity.x += VG.x * coef;
            velocity.y += VG.y * coef;
            velocity.z -= VG.z * coef;
        }
    }
    return velocity;
}


void main()
{
    int inboid = int(gl_GlobalInvocationID.x);

    vec4 oldpos = BoidBuffer.data[inboid].pos;
    vec4 oldvel = BoidBuffer.data[inboid].vel;
    vec4 velocity = vinf;
    vec4 newpos = vec4(0);


    if(RK==2)
    {
        vec4 v1 = vinf + getVelocity(oldpos);
        vec4 pos1 = oldpos + v1*dt;
        vec4 v2 = vinf + getVelocity(pos1);
        vec4 pos2 = oldpos + v2*dt;
        newpos = oldpos + v1*0.5*dt + v2*0.5*dt;
    }
    else if(RK==4)
    {
        vec4 v1 = vinf + getVelocity(oldpos);
        vec4 pos1 = oldpos + v1*0.5*dt;
        vec4 v2 = vinf + getVelocity(pos1);
        vec4 pos2 = oldpos + v2*0.5*dt;
        vec4 v3 = vinf + getVelocity(pos2);
        vec4 pos3 = oldpos + v2    *dt;
        vec4 v4 = vinf + getVelocity(pos3);
        newpos = oldpos + (v1 + v2*2.0 + v3*2.0 + v4)*dt/6.0;
    }
    else
    {
        // forward EULER
        velocity += getVelocity(oldpos);
        newpos = oldpos + velocity*dt;
    }

    bool bResetTrace = false;
    if(newpos.x>botright.x)
    {
        // raising the digits to get pseudo rand
        float randy = oldpos.y*1000.0f-floor(oldpos.y*1000.0f);
        float randz = oldpos.z*1000.0f-floor(oldpos.z*1000.0f);

        newpos.x = topleft.x;
        newpos.y = topleft.y + randy * (botright.y-topleft.y);
        newpos.z = topleft.z + randz * (botright.z-topleft.z);

        velocity = vinf;
        bResetTrace = true;
    }


    BoidBuffer.data[inboid].pos = newpos;
    BoidBuffer.data[inboid].vel = velocity;

    vec4 clr;
    if(HasUniColor==1)
        clr = UniformColor;
    else
    {
        float tau = length(velocity)/3.0/length(vinf);
        clr = vec4(glGetRed(tau), glGetGreen(tau), glGetBlue(tau), 1.0);
    }
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






