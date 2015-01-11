#version 130

uniform mat4 in_matrix;

in vec3 in_position;

void main()
{
    gl_Position = in_matrix * vec4(in_position, 1.0);
}
