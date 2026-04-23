#version 430

#define GROUP_SIZE 64

#define INFLUENCEDIST 3.0

#define MAXFORCE 0.03

#define RADIUS 1.0f

// todo: sync with value in gl3dView
#define TRACESEGS 32

uniform int cube; // cube==1, sphere otherwise

uniform float maxspeed;

uniform float width;
uniform float height;

uniform float cohesion;
uniform float separation;
uniform float alignment;
uniform float predatorf;

uniform int haspredator;

// Watch out for padding constraints of layouts 430/140 specifically for vec3:
// https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)
// using vec4 everywhere
// Vertex(4) / Velocity(4) / Color(4)
struct AttribData
{
    vec4 pos;
    vec4 vel;
    vec4 clr;
};


struct Trace
{
    vec4 vecs[TRACESEGS*2*2]; // TRACESEGS x 2 vertices x (pos4 + clr4)
};



layout(shared, binding = 0) buffer SSBO
{
    AttribData data[];
} outBuffer;

layout(shared, binding=1) buffer SSBO_1
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



vec3 force(int iboid)
{
    vec3 boidp = outBuffer.data[iboid].pos.xyz;
    vec3 boidv = outBuffer.data[iboid].vel.xyz;

    float neighbordist = width/INFLUENCEDIST;

    vec3 sum_c = vec3(0.0f);
    int cnt_c = 0;

    vec3 sum_a = vec3(0.0f);
    int cnt_a = 0;

    vec3 sum_s = vec3(0.0f);
    int cnt_s = 0;

    vec3 fa = vec3(0.0f);
    vec3 fc = vec3(0.0f);
    vec3 fs = vec3(0.0f);


    int size = int(gl_NumWorkGroups.x*gl_WorkGroupSize.x);
    if(haspredator==1) size--;

    for(int i=0; i<size; i++)
    {
        uint ib = i;
        vec3 pos = outBuffer.data[ib].pos.xyz;
        vec3 vel = outBuffer.data[ib].vel.xyz;

        if(ib != iboid)
        {
            float dist = distance(boidp, pos);

            if(dist<neighbordist)
            {
                sum_c += pos;
                cnt_c++;

                sum_a += vel;
                cnt_a++;

                if(dist>0.0)
                {
                    vec3 diff = normalize(boidp - pos);
                    diff *= 1.0/dist;
                    fs += diff;
                    cnt_s++;
                }
            }
        }
    }

    if(cnt_c>0)
    {
        sum_c *= 1.0/float(cnt_c);
        vec3 desired = normalize(sum_c - boidp);
        desired *= maxspeed;
        fc = desired - boidv;
        if(length(fc)>MAXFORCE)
        {
            fc = normalize(fc)*MAXFORCE;
        }
    }

    if(cnt_a>0)
    {
        sum_a *= 1.0/float(cnt_a);
        sum_a = normalize(sum_a) * maxspeed;
        fa = sum_a-boidv;
        if(length(fa)>MAXFORCE)
        {
            fa = normalize(fa)*MAXFORCE;
        }
    }

    if (cnt_s>0)  fs *= 1.0/float(cnt_s);
    if (length(fs)>0.0)
    {
        fs = normalize(fs) * maxspeed;
        fs = fs - boidv;
        if(length(fs)>MAXFORCE)
        {
            fs = normalize(fs)*MAXFORCE;
        }
    }

    vec3 predatorforce = vec3(0);
    if(haspredator==1)
    {
        int ipredator = int(gl_NumWorkGroups.x*gl_WorkGroupSize.x-1);
        vec3 predatorpos = outBuffer.data[ipredator].pos.xyz;
        float neighbordist = width/3.0f*predatorf;
        vec3 diff = boidp-predatorpos;
        float dist = length(diff);

        if(dist<=neighbordist) //  if the boids see the predator coming
        {
            float tau = (width-dist)/width;
            float fearfactor = tau*tau*tau/1.0;  // the closer the predator the greater the fear
            predatorforce = normalize(diff)*fearfactor;
        }
    }

    return fc*cohesion + fs*separation + fa*alignment + predatorforce*predatorf;
}


void main()
{
    int ipredator = int(gl_NumWorkGroups.x*gl_WorkGroupSize.x-1);

    int inboid = int(gl_GlobalInvocationID.x);
    //use second part of the buffer to avoid overwriting
    int outboid = inboid + int(gl_NumWorkGroups.x*gl_WorkGroupSize.x);

    vec4 oldpos = outBuffer.data[inboid].pos;
    vec4 oldvel = outBuffer.data[inboid].vel;
    vec4 accel = vec4(0);
    vec4 newvel = vec4(0);

    if(haspredator==1 && inboid == ipredator)
    {
        // accel/attraction is proportional to the size of the local flock
        vec4 predatorpos = outBuffer.data[ipredator].pos;
        float neighbordist = width/2.0;
        int size = int(gl_NumWorkGroups.x*gl_WorkGroupSize.x)-1;
        vec4 CoG = vec4(0);
        int cnt = 0;
        for(int i=0; i<size; i++)
        {
            if(distance(outBuffer.data[i].pos.xyz, predatorpos.xyz)<neighbordist)
            {
                CoG += outBuffer.data[i].pos;
                cnt++;
            }

        }
        if(cnt>0)
        {
            CoG *= 1.0/float(cnt);
            accel = (CoG - oldpos)/width;
            if(length(accel)>3.0*MAXFORCE) accel = normalize(accel)*3.0*MAXFORCE;
        }

        newvel = oldvel*0.9999 + accel;
        if(length(newvel)>1.5*maxspeed)   newvel = normalize(newvel)*1.5*maxspeed; // predator is 50% faster
    }
    else
    {
        accel = vec4(force(inboid),0);
        newvel = oldvel + accel;
        if(length(newvel)>maxspeed)       newvel = normalize(newvel)* maxspeed;
    }

    vec4 newpos = oldpos + newvel;

    // bounce off border
    if(cube==1)
    {
        if (newpos.x<-width+RADIUS) {newvel.x *= -1.0;  newpos.x = (-width+RADIUS) * 0.99;}
        if (newpos.x> width-RADIUS) {newvel.x *= -1.0;  newpos.x = ( width-RADIUS) * 0.99;}

        if (newpos.y<-width+RADIUS) {newvel.y *= -1.0;  newpos.y = (-width+RADIUS) * 0.99;}
        if (newpos.y> width-RADIUS) {newvel.y *= -1.0;  newpos.y = ( width-RADIUS) * 0.99;}

        if (newpos.z<-height+RADIUS) {newvel.z *= -1.0;  newpos.z = (-height+RADIUS) * 0.99;}
        if (newpos.z> height-RADIUS) {newvel.z *= -1.0;  newpos.z = ( height-RADIUS) * 0.99;}
    }
    else
    {
        float amp = sqrt(newpos.x*newpos.x + newpos.y*newpos.y + newpos.z*newpos.z);
        vec4 uradial = newpos/amp;
        if(amp>width-2.0*RADIUS)
        {
            newpos = newpos - uradial * 2.0*RADIUS;
            newvel = reflect(newvel, uradial);
        }
    }

    outBuffer.data[outboid].pos = newpos;
    outBuffer.data[outboid].vel = newvel;

    float tau = length(newvel)/maxspeed;
    if(haspredator==1 && inboid==ipredator)
    {
       tau = 1.0f;
    }

    vec4 clr = vec4(glGetRed(tau), glGetGreen(tau), glGetBlue(tau), 1.0f);

    outBuffer.data[outboid].clr = clr;

    //shift traces 1 segment
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






