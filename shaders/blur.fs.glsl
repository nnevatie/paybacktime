#version 150

// Uniforms
uniform sampler2D texDepth;
uniform sampler2D texAo;

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
    vec2 texelSize = 1.0 / vec2(textureSize(texAo, 0));
    float d        = texture(texDepth, ib.uv).r;
    int samples    = 0;
    float sum      = 0.0f;
    for (int t = -2; t < 2; ++t)
        for (int s = -2; s < 2; ++s)
        {
            vec2 offset = vec2(float(s), float(t)) * texelSize;
            if (abs(d - texture(texDepth, ib.uv + offset).r) < 0.0025 * d)
            {
                sum += texture(texAo, vec2(ib.uv + offset)).r;
                ++samples;
            }
        }
    sum  /= samples > 0 ? samples : 1;
    color = vec4(sum, sum, sum, 1.0);
}
