#version 150

// Uniforms
uniform sampler2D tex;
uniform vec2      invDirSize;
uniform int       radius;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

vec4 blur(vec2 uv, float r, vec4 cc, inout float sw)
{
    vec4  c       = texture(tex, uv);
    float sigma   = 0.5 * radius;
    float falloff = 1.0 / (2.0 * sigma * sigma);
    float w       = exp2(-r * r * falloff);
    sw           += w;
    return w * c;
}

void main()
{
    vec4  cc = texture(tex, ib.uv);
    vec4  sc = cc;
    float sw = 1.0;
    for (float r = 1; r <= radius; ++r)
    {
        vec2 o = invDirSize * r;
        sc    += blur(ib.uv - o, r, cc, sw);
        sc    += blur(ib.uv + o, r, cc, sw);
    }
    color = vec4(sc / sw);
}
