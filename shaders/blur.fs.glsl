#version 150

// Uniforms
uniform sampler2D tex;

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
    vec2 texelSize = 1.0 / vec2(textureSize(tex, 0));
    float sum = 0.0f;
    for (int t = -2; t < 2; ++t)
        for (int s = -2; s < 2; ++s)
            sum += texture(tex, vec2(ib.uv + vec2(float(s), float(t)) *
                                               texelSize)).r;
    sum  /= 4.0 * 4.0;
    color = vec4(sum, sum, sum, 1.0);
    //color = texture(tex, ib.uv);
}
