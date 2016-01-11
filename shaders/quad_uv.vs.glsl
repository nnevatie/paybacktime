#version 150

// Uniforms
uniform float tanHalfFov;
uniform float aspectRatio;
uniform float z;

// Input
in vec3 position;
in vec2 uv;

// Output
out Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ob;

void main()
{
    ob.uv       = uv;
    ob.viewRay  = vec3(position.x * tanHalfFov * aspectRatio,
                       position.y * tanHalfFov,
                       -1.0);
    gl_Position = vec4(position.xy, z, 1.0);
}
