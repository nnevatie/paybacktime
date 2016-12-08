#version 150

// Uniforms
uniform sampler3D density;
uniform sampler3D emission;
uniform float     z;
uniform float     exp;
uniform float     ambient;
uniform float     attMin;
uniform float     k0;
uniform float     k1;
uniform float     k2;
uniform vec3      cs;

// Const
ivec3 size = ivec3(textureSize(density, 0));

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 light;

void swap(inout ivec3 v0, inout ivec3 v1)
{
    ivec3 t = v0;
    v0 = v1;
    v1 = t;
}

float vis(ivec3 p0, ivec3 p1)
{
    // Make results symmetrical between endpoints
    if (p0.y > p1.y) swap(p0, p1);

    ivec3 d = p1 - p0;
    int   n = abs(d.x) > abs(d.y) ? abs(d.x) : abs(d.y);
    vec3  s = vec3(d) / n;

    float v = 1.0;
    vec3 p  = vec3(p0) + s + 0.5;

    for (int i = 0; i < n - 1 && v > 0.0; ++i, p += s)
        v -= texelFetch(density, ivec3(p), 0).r;

    return max(0.0, v);
}

void main(void)
{
    ivec3 p0 = ivec3(gl_FragCoord.xy, z);
    vec3 l   = vec3(0);

    for (int ez = 0; ez < size.z; ++ez)
        for (int ey = 0; ey < size.y; ++ey)
            for (int ex = 0; ex < size.x; ++ex)
            {
                ivec3 p1 = ivec3(ex, ey, ez);
                vec3 e   = texelFetch(emission, p1, 0).rgb;
                if (length(e) > 0.0)
                {
                    float d   = distance(cs * vec3(p0), cs * vec3(p1));
                    float r   = sqrt(1.0 / (k2 * attMin));
                    if (d <= r)
                    {
                        float att = 1.0 / (k0 + k1 * d + k2 * d * d);
                        if (att > attMin)
                        {
                            float v = vis(p1, p0);
                            l      += v * v * exp * e * att;
                        }
                    }
                }
            }

    float d = texelFetch(density, p0, 0).r;
    light   = vec4(l, 1);
}
