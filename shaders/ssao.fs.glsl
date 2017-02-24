#version 150

#define RADIUS           10.0
#define KERNEL_SIZE      32
#define POW              4

// Uniforms
uniform sampler2D texDepth;
uniform sampler2D texNormal;
uniform sampler2D texNoise;
uniform vec3      kernel[KERNEL_SIZE];
uniform vec2      noiseScale;
uniform mat4      p;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

void main(void)
{
    vec3 normal    = texture(texNormal, ib.uv).xyz;
    vec3 fragPos   = texture(texDepth, ib.uv).r * -ib.viewRay - 0.5 * normal;
    vec3 randomVec = texture(texNoise, ib.uv * noiseScale).xyz;
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn       = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = 0; i < KERNEL_SIZE; ++i)
    {
        vec3 ray    = tbn * kernel[i];
        vec3 sample = fragPos - ray * RADIUS;

        vec4 offset = vec4(sample, 1.0);
        offset      = p * offset;
        offset.xy  /= offset.w;
        offset.xy   = offset.xy * 0.5 + 0.5;

        float sampleDepth = texture(texDepth, offset.xy).r;
        float rangeCheck  = smoothstep(0.0, 1.0, 0.25 * RADIUS /
                                                 abs(fragPos.z - sampleDepth));

        occlusion        += rangeCheck * step(sampleDepth, sample.z);
    }
    color = vec4(pow(1.0 - (occlusion / KERNEL_SIZE), POW));
}
