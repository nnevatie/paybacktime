#version 150

// Uniforms
uniform sampler2D texColor;

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
    color = texture(texColor, ib.uv);
}
