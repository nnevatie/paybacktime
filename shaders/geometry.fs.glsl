#version 150

// Uniforms
uniform mat4 mv;
uniform mat4 p;
uniform sampler2D albedo;

// Input
in Block
{
    vec3 viewPos;
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
ib;

// Output
out vec4 posDepth;
out vec3 normal;
out vec4 color;

float linearDepth(float depth, mat4 proj)
{
    float near = proj[3][2] / (proj[2][2] - 1);
    float far  = proj[3][2] / (proj[2][2] + 1);
    float z    = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    posDepth = vec4(ib.viewPos, linearDepth(gl_FragCoord.z, p));
    normal   = normalize(ib.normal);
    color    = texture(albedo, ib.uv);
}
