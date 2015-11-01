#version 150

// Uniforms
uniform mat4 mv;
uniform mat4 p;
uniform sampler2D texAlbedo;

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

float edge(vec3 bc)
{
    vec3 d  = fwidth(bc);
    vec3 a3 = smoothstep(vec3(0.0), d * 1.0, bc);
    return min(min(a3.x, a3.y), a3.z);
}

void main()
{
    posDepth = vec4(ib.viewPos, linearDepth(gl_FragCoord.z, p));
    normal   = normalize(ib.normal);
    vec4 alb = texture(texAlbedo, ib.uv);
    color    = vec4(mix(alb - 0.25, alb, edge(ib.bc)).rgb, 1.0);
}
