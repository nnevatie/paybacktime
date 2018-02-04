#version 150

// Uniforms
uniform sampler2D texDepth;
uniform sampler3D texDensity;
uniform mat4      w;
uniform vec3      camPos;
uniform vec3      boundsMin;
uniform vec3      boundsSize;
uniform int       sampleCount;

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

vec3 density(vec3 start)
{
    vec3 ray  = normalize(camPos - start);
    vec3 end  = camPos - 1000.0 * ray;
    vec3 uvw0 = worldUvw(camPos, boundsMin, boundsSize);
    vec3 uvw1 = worldUvw(end,    boundsMin, boundsSize);
    vec3 uvws = (uvw1 - uvw0) / sampleCount;

    vec4 s = vec4(0.0);
    for (int i = 0; i < sampleCount; ++i)
    {
        vec4 d = texture(texDensity, uvw0);
        if (d.a != 0.0)
        {
            s.rgb += abs(d.a) * normalize(d.rgb);
            s.a++;
        }
        uvw0  += uvws;
    }
    return s.a > 0.0 ? s.rgb / s.a : vec3(0.0);
}

void main(void)
{
    vec3 worldPos = world(texDepth, ib.uv, w);
    color = vec4(density(worldPos), 1.0);
}
