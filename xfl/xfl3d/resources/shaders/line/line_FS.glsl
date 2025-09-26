#version 330

uniform int pattern;   // an integer betwween 0 and 0xFFFF representing the bitwise pattern
uniform int nPatterns; // the number of patterns/unit length of the viewport, typically 200-300 for good pattern density
uniform float clipPlane0; // defined in view-space
uniform int HasUniColor = 1;
uniform vec4 UniformColor;

in float texCoord;
in vec3 vPosition;
in vec4 GeomColor;

layout(location=0) out vec4 fragColor;


void main(void)
{
    if (vPosition.z > clipPlane0) {
        discard;
        return;
    }

    // use 4 bytes for the masking pattern
    // map the texture coordinate to the interval [0,2*8[
    uint bitpos = uint(round(texCoord * nPatterns)) % 16U;
    // move a unit bit 1U to position bitpos so that
    // bit is an integer between 1 and 1000 0000 0000 0000 = 0x8000
    uint bit = (1U << bitpos);

    // test the bit against the masking pattern
    // example:   up = 0xFFFF
    //  Line::SOLID:       pattern = 0xFFFF;  // = 1111 1111 1111 1111 = solid pattern
    //  Line::DASH:        pattern = 0x3F3F;  // = 0011 1111 0011 1111
    //  Line::DOT:         pattern = 0x6666;  // = 0110 0110 0110 0110
    //  Line::DASHDOT:     pattern = 0xFF18;  // = 1111 1111 0001 1000
    //  Line::DASHDOTDOT:  pattern = 0x7E66;  // = 0111 1110 0110 0110
    uint up = uint(pattern);

    // discard the bit if it doesn't match the masking pattern
    if ((up & bit) == 0U) discard;

    if(HasUniColor==1)
        fragColor = UniformColor;
    else
        fragColor = GeomColor; // from the Geometry Shader
}
