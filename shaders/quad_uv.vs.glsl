#version 150

// Input
in vec3 position;
in vec2 uv;

// Output
out Block
{
    vec2 uv;
}
ob;

void main()
{
    ob.uv       = uv;
    gl_Position = vec4(position, 1.0);
}
