#version 150

// Uniforms
uniform sampler2D density;
uniform float     z;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 light;

void main(void)
{
    light = vec4(1);
}
