#version 150

// Uniforms
uniform sampler2D tex;
uniform bool      horizontal;
uniform int       radius = 5;
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
    vec2 ts = 1.0 / textureSize(tex, 0);
    vec2 d  = horizontal ? vec2(ts.x, 0) : vec2(0, ts.y);

    vec3 result;
    for(int i = -radius; i <= radius; ++i)
        result += texture(tex, ib.uv + i * d).rgb;

    color = vec4(result / (radius * 2 + 1), 1.0);
}
