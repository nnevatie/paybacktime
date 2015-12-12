#version 150

// Uniforms
uniform mat4 mvp;

// Input
in vec3 position;
in vec3 normal;
in vec2 uv;

// Output
// gl_Position

void main()
{
    gl_Position = mvp * vec4(position, 1.0);
}
