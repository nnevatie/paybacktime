#version 150

// Uniforms
uniform sampler2D tex;
uniform vec4      clip;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out float color;

float linearDepth(float depth, vec4 c)
{
    return c[0] / (c[1] * depth + c[2]);
}

void main()
{
    float depth = texture(tex, ib.uv).r;
    color       = linearDepth(depth, clip);
}
