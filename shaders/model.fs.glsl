#version 150

// Uniforms
uniform sampler2D texAlbedo;

// Input
in Block
{
    vec3 normal;
    vec2 uv;
}
ib;

// Output
out vec4 color;

void main()
{
    vec4 alb = texture(texAlbedo, ib.uv);
    color    = alb;
}
