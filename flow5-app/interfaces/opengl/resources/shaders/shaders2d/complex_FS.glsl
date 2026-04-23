#version 330

#define PI 3.141592654
#define cmul(a, b) vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)
#define cdiv(a, b) vec2((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y), (a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y))
#define cadd(a, b) vec2(a.x+b.x, a.y+b.y)
#define csub(a, b) vec2(a.x-b.x, a.y-b.y)


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

float hueToRgb(float p, float q, float t)
{
    if (t < 0.0) t += 1;
    if (t > 1.0) t -= 1;
    if (t < 1.0/6.0) return p + (q - p) * 6.0 * t;
    if (t < 1.0/2.0) return q;
    if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
    return p;
}


// adapted from https://en.wikipedia.org/wiki/HSL_color_space.
vec3 hslToRgb(float h, float s, float l) {
    float r, g, b;

    if (s <= 0)
    {
        r = g = b = l; // achromatic
    }
    else {
        float q = l < 0.5 ? l * (1.0 + s) : l + s - l*s;
        float p = 2.0*l - q;
        r = hueToRgb(p, q, h + 1.0/3.0);
        g = hueToRgb(p, q, h);
        b = hueToRgb(p, q, h - 1.0/3.0);
    }

    return vec3(r,g,b);
}


vec2 f(vec2 z)
{
/*    vec2 res = (cmul(z,z) - 1) * cadd(z, vec2(-2,-1));
    res = cdiv(res, cadd(cmul(z,z), vec2(2,2)));

    return res;*/

    vec2 root0 = vec2(cos(0), sin(0));
    vec2 root1 = vec2(cos(2.0*PI/3.0), sin(2.0*PI/3.0));
    vec2 root2 = vec2(cos(-2.0*PI/3.0), sin(-2.0*PI/3.0));

    vec2 res = csub(z, root0);
    res = cmul(res, csub(z, root1));
    res = cmul(res, csub(z, root2));
    return res;
}




void main(void)
{
    vec2 z = pos;

    z = f(z);

    float argz = atan(z.y, z.x); // in [-PI/PI]
    float r = sqrt(z.x*z.x+z.y*z.y);
    float L = 2.0/PI * atan(r);
    float tau  = (argz + PI)/2.0/PI;
    vec3 hsl = hslToRgb(tau, 1.0, 1.0-L);

/*    float tau  = (argz + PI)/2.0/PI;
    float red   = glGetRed(tau);
    float green = glGetGreen(tau);
    float blue  = glGetBlue(tau);*/

    FragmentColor = vec4(hsl,1.0f);

}



