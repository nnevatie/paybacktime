#version 150

// Uniforms
uniform sampler2D tex;

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

void main()
{
    color = texture(tex, ib.uv);
/*
    float z = texture(tex, ib.uv).r;
    float n = 0.01;
    float f = 400.0;
    float c = (2.0 * n) / (f + n - z * (f - n));
    color   = vec4(c, c, c, 1);
*/
}
