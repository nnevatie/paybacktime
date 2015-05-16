#version 150
#extension GL_ARB_separate_shader_objects : enable

uniform mat4 transform;

in vec4 posAo;

layout (location = 0)
out float vertexAo;

void main()
{
    vertexAo    = posAo.w;
    gl_Position = transform * vec4(posAo.xyz, 1);
}
