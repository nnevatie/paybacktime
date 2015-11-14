#version 150

// Uniforms
uniform sampler2D tex;
uniform bool      horizontal;
uniform float     weight[5] = float[]
                  (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

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
    vec3 result = texture(tex, ib.uv).rgb * weight[0];
    for(int i = 1; i < 5; ++i)
    {
        result += texture(tex, ib.uv - i * d).rgb * weight[i];
        result += texture(tex, ib.uv + i * d).rgb * weight[i];
    }
    color = vec4(result, 1.0);
}
