#version 330

#define cmul(a, b) vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)
#define cdiv(a, b) vec2((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y), (a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y))
#define cadd(a, b) vec2(a.x+b.x, a.y+b.y)
#define csub(a, b) vec2(a.x-b.x, a.y-b.y)

uniform int maxiters;
uniform float tolerance;

uniform int nroots;

uniform vec4 color0;
uniform vec4 color1;
uniform vec4 color2;
uniform vec4 color3;
uniform vec4 color4;

uniform vec2 root0;
uniform vec2 root1;
uniform vec2 root2;
uniform vec2 root3;
uniform vec2 root4;

in vec2 pos;


layout(location = 0) out vec4 FragmentColor;


vec2 polynomial(vec2 z)
{
    vec2 res = csub(z, root0);
    res = cmul(res, csub(z, root1));
    res = cmul(res, csub(z, root2));
    if(nroots>3)
    {
        res = cmul(res, csub(z, root3));
        res = cmul(res, csub(z, root4));
    }
    return res;
}


vec2 derivative(vec2 z)
{
    vec2 z0 = csub(z,root0);
    vec2 z1 = csub(z,root1);
    vec2 z2 = csub(z,root2);
    vec2 z3 = csub(z,root3);
    vec2 z4 = csub(z,root4);
    if(nroots==3)
    {
        vec2 res = cmul(z0,z1) +
                   cmul(z1,z2) +
                   cmul(z2,z0);
        return res;
    }
    else
    {
        vec2 res = cmul(cmul(cmul(z1, z2), z3), z4) +
                   cmul(cmul(cmul(z0, z2), z3), z4) +
                   cmul(cmul(cmul(z0, z1), z3), z4) +
                   cmul(cmul(cmul(z0, z1), z2), z4) +
                   cmul(cmul(cmul(z0, z1), z2), z3);
        return res;
    }
}


void main(void)
{
    vec4 colors[5];
    colors[0] = color0;
    colors[1] = color1;
    colors[2] = color2;
    colors[3] = color3;
    colors[4] = color4;

    vec2 roots[5];
    roots[0] = root0;
    roots[1] = root1;
    roots[2] = root2;
    roots[3] = root3;
    roots[4] = root4;

    vec2 z = pos;

    for(int iteration=0; iteration<maxiters; iteration++)
    {
        z = z - cdiv(polynomial(z), derivative(z));

        for(int i=0; i<nroots; i++)
        {
            vec2 difference = z - roots[i];

            if (abs(difference.x)<tolerance && abs(difference.y)<tolerance)
            {
                FragmentColor = colors[i] * min(16.0, float(iteration))/16.0;
                return;
            }
        }
    }

    FragmentColor = vec4(0,0,0,1);
}



