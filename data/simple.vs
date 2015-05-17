#version 150
#extension GL_ARB_separate_shader_objects : enable

uniform mat4 transform;

in vec3 position;
in vec3 normal;
in vec2 uv;

out vec3 normalV;
out vec2 uvV;

void main()
{
    normalV     = normal;
    uvV         = uv;
    gl_Position = transform * vec4(position.xyz, 1);
}
