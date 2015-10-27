#version 150

// Uniforms
uniform sampler2D tex;

// Input
in Block
{
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
}
