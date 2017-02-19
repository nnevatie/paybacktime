#version 150

// Uniforms
uniform sampler2D tex;
uniform bool      horizontal;

uniform float     offset[3] = float[](0.0,          1.3846153846, 3.2307692308);
uniform float     weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

/*
uniform float     offset[5] = float[](0.0, 1.0, 2.0, 3.0, 4.0);
uniform float     weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
*/

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
    vec2 ts     = 1.0 / textureSize(tex, 0);
    vec2 d      = horizontal ? vec2(ts.x, 0) : vec2(0, ts.y);
    vec4 result = texture(tex, ib.uv) * weight[0];
    for(int i = 1; i < 3; ++i)
    {
        result += texture(tex, ib.uv - d * offset[i]) * weight[i];
        result += texture(tex, ib.uv + d * offset[i]) * weight[i];
    }
    color = result;
}
