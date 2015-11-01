#version 150

// Uniforms
uniform mat4 mvp;

// Input
in vec3 position;

// Output

void main()
{
    gl_Position = mvp * vec4(position, 1.0);
}
