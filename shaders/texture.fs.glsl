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
input;

// Output
out vec4 frag;

void main()
{
    frag = texture(tex, input.uv);
}
