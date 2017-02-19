#version 150

// Uniforms
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
    float sum      = 0.0f;
    for (int t = -2; t < 2; ++t)
        for (int s = -2; s < 2; ++s)
        {
            vec2 offset = vec2(float(s), float(t)) * texelSize;
            sum += texture(texAo, vec2(ib.uv + offset)).r;
        }

    sum  /= 4 * 4;
    color = vec4(sum, sum, sum, 1.0);
}
