#version 150

// Uniforms
uniform sampler2D texDepth;
uniform sampler3D texGi;
uniform mat4      w;
uniform vec3      camPos;
uniform vec3      boundsMin;
uniform vec3      boundsSize;

// Const
const int   SCATTER_STEPS = 20;
const float SCATTER_DIST  = 100.0;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

// Externals
vec3 world(sampler2D depth, vec2 uv, mat4 w);
vec3 worldUvw(vec3 pos, vec3 boundsMin, vec3 boundsMax);

vec3 scattering(vec3 start)
{
    vec3 ray  = normalize(camPos - start);
    vec3 end  = start + ray * SCATTER_DIST;
    vec3 uvw0 = worldUvw(start, boundsMin, boundsSize);
    vec3 uvw1 = worldUvw(end, boundsMin, boundsSize);
    vec3 uvws = (uvw1 - uvw0) / SCATTER_STEPS;

    float weight = 0.05;
    vec3 scatter = vec3(0.0);
    for (int i = 0; i < SCATTER_STEPS && uvw0.z < 1.05; ++i)
    {
        vec3 gi  = texture(texGi, uvw0).rgb;
        scatter += weight * pow(gi, vec3(1.75));
        uvw0    += uvws;
    }
    return scatter;
}

void main(void)
{
    vec3 worldPos = world(texDepth, ib.uv, w);
    color = vec4(scattering(worldPos), 1.0);
}
