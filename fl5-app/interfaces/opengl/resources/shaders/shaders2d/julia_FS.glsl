#version 330

uniform int julia;
uniform vec2 param; // the Julia  parameter

uniform float tau;

uniform int maxiters;
uniform float maxlength;

in vec2 pos;

layout(location = 0) out vec4 FragmentColor;



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



vec2 square(vec2 v)
{
    return vec2(v.x*v.x-v.y*v.y, 2.0*v.x*v.y);
}


void main(void)
{
    vec2 pt = vec2(0,0);
    vec2 startpt = vec2(0,0);

    if(julia==1)
    {
        pt=pos;
        startpt = param;
    }
    else
    {
        startpt = pos;
    }

    int iter=0;
    do
    {
        pt = square(pt) + startpt;
        iter++;
    }
    while(length(pt)<maxlength && iter<maxiters);

    if(iter == maxiters) FragmentColor = vec4(0,0,0,1);
    else
    {
        FragmentColor = min(32.0, float(iter))/32.0 * vec4(glGetRed(tau), glGetGreen(tau), glGetBlue(tau), 1.0);
    }
}


