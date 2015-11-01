#version 150

// Uniforms
uniform vec4 albedo;

// Input

// Output
out vec4 color;

void main()
{
    color = albedo;
}
