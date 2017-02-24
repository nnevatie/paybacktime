#version 150

// Uniforms
uniform sampler2D texColor;
uniform sampler2D texDepth;
uniform vec2      invDirSize;
uniform int       radius;
uniform float     sharpness;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

vec4 blur(vec2 uv, float r, vec4 cc, float cd, inout float sw)
{
    vec4  c       = texture(texColor, uv);
    float d       = texture(texDepth, uv).r;
    float sigma   = 0.5 * radius;
    float falloff = 1.0 / (2.0 * sigma * sigma);
    float dd      = (d - cd) * sharpness;
    float w       = exp2(-r * r * falloff - dd * dd);
    sw           += w;
    return w * c;
}

void main()
{
    vec4  cc = texture(texColor, ib.uv);
    float cd = texture(texDepth, ib.uv).r;
    vec4  sc = cc;
    float sw = 1.0;
    for (float r = 1; r <= radius; ++r)
    {
        vec2 o = invDirSize * r;
        sc    += blur(ib.uv - o, r, cc, cd, sw);
        sc    += blur(ib.uv + o, r, cc, cd, sw);
    }
    color = vec4(sc.rgb / sw, cc.a);
}
