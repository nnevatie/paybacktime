#version 150

#define RADIUS           2.0
#define KERNEL_SIZE      32
#define POW              2

// Uniforms
uniform sampler2D texPosDepth;
uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texNoise;
uniform vec3      kernel[KERNEL_SIZE];
uniform vec2      noiseScale;
uniform mat4      p;

// Input
in Block
{
    vec2 uv;
}
ib;

// Output
out vec4 color;

void main(void)
{
    vec3 fragPos   = texture(texPosDepth, ib.uv).xyz;
    vec3 normal    = texture(texNormal, ib.uv).xyz;
    vec3 randomVec = texture(texNoise, ib.uv * noiseScale).xyz;
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn       = mat3(tangent, bitangent, normal);

    // Offset from surface, prevent self-occlusion
    fragPos       += 0.5 * normal;

    float occlusion = 0.0;
    for(int i = 0; i < KERNEL_SIZE; ++i)
    {
        vec3 ray    = tbn * kernel[i];
        vec3 sample = fragPos + ray * RADIUS;

        vec4 offset = vec4(sample, 1.0);
        offset      = p * offset;
        offset.xy  /= offset.w;
        offset.xy   = offset.xy * 0.5 + 0.5;

        float sampleDepth = -texture(texPosDepth, offset.xy).w;
        float range       = abs(fragPos.z - sampleDepth);
        float rangeCheck  = smoothstep(0.0, 1.0, RADIUS / range);

        occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;
    }
    color = vec4(pow(1.0 - (occlusion / KERNEL_SIZE), POW));
}
