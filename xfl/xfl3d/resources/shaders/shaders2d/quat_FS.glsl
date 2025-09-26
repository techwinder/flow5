#version 330

#define PI 3.141592654

uniform int julia;
uniform int slice;
uniform vec4 seed;
uniform vec2 slicer;
uniform vec4 color;
uniform int maxiters;
uniform float maxlength;

in vec2 pos;

layout(location = 0) out vec4 FragmentColor;




vec4 mult(vec4 qt0, vec4 qt1)
{
    vec4 prod;

    prod.w = qt0.w*qt1.w  - qt0.x*qt1.x - qt0.y*qt1.y - qt0.z*qt1.z;

    prod.x = qt0.w*qt1.x + qt0.x*qt1.w  + qt0.y*qt1.z - qt0.z*qt1.y;
    prod.y = qt0.w*qt1.y + qt0.y*qt1.w  + qt0.z*qt1.x - qt0.x*qt1.z;
    prod.z = qt0.w*qt1.z + qt0.z*qt1.w  + qt0.x*qt1.y - qt0.y*qt1.x;

    return prod;
}


void main(void)
{
    vec4 quat = vec4(0,0,0,0);

    if(slice==0)
    {
        // x-y
        quat.z = pos.x;
        quat.w = pos.y;
        quat.x = slicer.x;
        quat.y = slicer.y;
    }
    else if(slice==1)
    {
        //x-z
        quat.y = pos.x;
        quat.w = pos.y;
        quat.x = slicer.x;
        quat.z = slicer.y;
    }
    else if(slice==2)
    {
        //y-z
        quat.x = pos.x;
        quat.w = pos.y;
        quat.y = slicer.x;
        quat.z = slicer.y;
    }
    else if(slice==3)
    {
        //x-w
        quat.y = pos.x;
        quat.z = pos.y;
        quat.x = slicer.x;
        quat.w = slicer.y;
    }
    else if(slice==4)
    {
        //y-w
        quat.x = pos.x;
        quat.z = pos.y;
        quat.y = slicer.x;
        quat.w = slicer.y;
    }
    else if(slice==5)
    {
        //z-w
        quat.x = pos.x;
        quat.y = pos.y;
        quat.z = slicer.x;
        quat.w = slicer.y;
    }
    else if(slice==6)
    {
        //theta-phi
        float angle = slicer.x * PI;  //in [-PI, PI]
        float length = slicer.y+1.0;  //in [0.0, 2.0f]

        float phi   = pos.x * PI;
        float theta = pos.y * PI/2.0;

        vec3 N = vec3(cos(phi) * cos(theta), sin(phi)*cos(theta), sin(theta)) * length;

        quat.w = cos(angle/2.0);
        quat.x = N.x * sin(angle/2.0);
        quat.y = N.y * sin(angle/2.0);
        quat.z = N.z * sin(angle/2.0);
    }

    vec4 quat0 = vec4(0,0,0,0);

    if(julia==1)
    {
        quat0 = seed;
    }
    else
    {
        // mandelbrot 4d
        quat0 = quat;
    }

    int iter=0;
    do
    {
        quat = mult(quat, quat) + quat0;
    }
//    while(length(quat.xyz)<maxlength && ++iter<maxiters);
    while(length(quat)<maxlength && ++iter<maxiters);

    if(iter == maxiters) FragmentColor = vec4(0,0,0,1);
    else
    {
        FragmentColor = min(32.0, float(iter))/32.0 * color;
//        FragmentColor =  float(iter)/float(maxiters) * color;
//        FragmentColor =  float(iter)/32.0 * color;
    }
}


