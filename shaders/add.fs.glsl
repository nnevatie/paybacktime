#version 150

// Uniforms
uniform sampler2D tex0;
uniform sampler2D tex1;

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
    color = texture(tex0, ib.uv) + texture(tex1, ib.uv);
}
