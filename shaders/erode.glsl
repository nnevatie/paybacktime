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

void main()
{
    vec4 er = texture(tex, ib.uv);
    for (float r = 1; r <= radius; ++r)
    {
        vec2 o = invDirSize * r;
        er = min(er, texture(tex, ib.uv - o));
        er = min(er, texture(tex, ib.uv + o));
    }
    color = er;
}
