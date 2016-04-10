#version 150

// Uniforms
uniform sampler2D texColor;

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
    vec4 c = texture(texColor,  ib.uv);
    color  = smoothstep(vec4(1.25), vec4(8.0), c);
}
