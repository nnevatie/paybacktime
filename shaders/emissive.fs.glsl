#version 150

// Uniforms
uniform sampler2D texColor;
uniform sampler2D texLight;

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
    color = texture(texColor, ib.uv) * texture(texLight, ib.uv).b;
}
