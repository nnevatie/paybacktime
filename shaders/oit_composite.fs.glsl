#version 150

// Uniforms
uniform sampler2D tex0;
uniform sampler2D tex1;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

void main(void)
{
    vec4 accum   = texture2D(tex0, ib.uv);
    float reveal = accum.a;
    accum.a      = texture2D(tex1, ib.uv).r;
    if (reveal >= 1.0) discard;
    color        = vec4(accum.rgb / clamp(accum.a, 1e-4, 5e4), reveal);
}
