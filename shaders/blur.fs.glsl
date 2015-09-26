#version 150

// Uniforms
uniform sampler2D texColor;
uniform sampler2D texSsao;
uniform vec2      texelStep;

// Input
in Block
{
    vec3 eye;
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
ib;

// Output
out vec4 color;

void main(void)
{
    float sum = 0.0f;
    for (int t = -2; t <= 2; t++)
        for (int s = -2; s <= 2; s++)
            sum += texture(texSsao,
                           vec2(ib.uv.s + float(s) * texelStep.s,
                                ib.uv.t + float(t) * texelStep.t)).r;
    sum  /= 25.0;
    //color = vec4(sum, sum, sum, 1.0);
    color = texture(texColor, ib.uv) * sum;
    //color = texture(texColor, ib.uv);
}
