#version 150

#define KERNEL_SIZE      32
#define CAP_MIN_DISTANCE 0.0005
#define CAP_MAX_DISTANCE 0.25

// Uniforms
uniform sampler2D texColor;
uniform sampler2D texNormal;
uniform sampler2D texDepth;
uniform sampler2D texNoise;
uniform vec3      kernel[KERNEL_SIZE];
uniform vec2      noiseScale;
uniform mat4      invP;
uniform mat4      p;
uniform float     r;

// Input
in Block
{
    vec3 eye;
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
ib;

// Output
out vec4 color;

vec4 viewPos(vec2 uv)
{
    float x = uv.s * 2.0 - 1.0;
    float y = uv.t * 2.0 - 1.0;
    float z = texture(texDepth, uv).r * 2.0 - 1.0;
    vec4 p  = vec4(x, y, z, 1.0);
    vec4 v  = invP * p;
    v      /= v.w;
    return v;
}

void main(void)
{
    vec4 posView       = viewPos(ib.uv);
    vec3 normalView    = normalize(texture(texNormal, ib.uv).
                                   xyz * 2.0 - 1.0);
    vec3 randomVector  = normalize(texture(texNoise,
                                   ib.uv * noiseScale).xyz * 2.0 - 1.0);
    vec3 tangentView   = normalize(randomVector - dot(
                                   randomVector, normalView) * normalView);
    vec3 bitangentView = cross(normalView, tangentView);
    mat3 kernelMatrix  = mat3(tangentView, bitangentView, normalView);

    float occ = 0.0;
    for (int i = 0; i < KERNEL_SIZE; ++i)
    {
        vec3 vv    = kernelMatrix * kernel[i];
        vec4 pv    = posView + r * vec4(vv, 0.0);
        vec4 pndc  = p * pv;
        pndc      /= pndc.w;
        vec2 puv   = pndc.xy * 0.5 + 0.5;
        float sndc = texture(texDepth, puv).r * 2.0 - 1.0;
        float d    = pndc.z - sndc;
        if (d > CAP_MIN_DISTANCE && d < CAP_MAX_DISTANCE)
            occ += 1.0;
    }
    occ   = 1.0 - (occ / KERNEL_SIZE);
    color = vec4(occ, occ, occ, 1.0);
}
