#version 150

// Uniforms
uniform sampler2D texAo;

// Input
in Block
{
    vec2 uv;
}
ib;

// Output
out vec4 color;

void main(void)
{
    vec2 texelSize = 1.0 / vec2(textureSize(texAo, 0));
    float sum = 0.0f;
    for (int t = -2; t < 2; ++t)
        for (int s = -2; s < 2; ++s)
            sum += texture(texAo, vec2(ib.uv + vec2(float(s), float(t)) *
                                               texelSize)).r;
    sum  /= 4.0 * 4.0;
    color = vec4(sum, sum, sum, 1.0);
    //color = texture(texAo, ib.uv);
}
