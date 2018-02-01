#version 150

// Uniforms
uniform sampler3D      density;
uniform sampler3D      emission;
uniform sampler2D      horizon;
uniform isamplerBuffer lightSrc;

uniform int            wz;
uniform int            lsc;
uniform float          attMin;
uniform float          k0;
uniform float          k1;
uniform float          k2;
uniform vec3           cs;

// Const
ivec3 size             = ivec3(textureSize(density, 0));
const int HORZ_SAMPLES = 8;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Outputs
out vec4 light;
out vec4 incidence;

// Externals
float vis(sampler3D tex, ivec3 p0, ivec3 p1, inout vec3 e, float el);
float cmin(vec3 v);
float cmax(vec3 v);

float distance2(vec3 v0, vec3 v1)
{
    vec3 d = v0 - v1;
    return d.x * d.x + d.y * d.y + d.z * d.z;
}

void main(void)
{
    // Position, light, incidence, pulse
    ivec3 p0 = ivec3(gl_FragCoord.xy, wz);
    vec3 l   = vec3(0);
    vec3 i   = vec3(0);
    vec4 p   = vec4(0);

    // Light sources
    float dmin = 10e6;
    for (int li = 0; li < lsc; ++li)
    {
        ivec4 ls = texelFetch(lightSrc, li);
        vec2  lp = vec2((ls.w & 0xff) / 255.0, ((ls.w >> 8) & 0xff) / 255.0);
        ivec3 p1 = ivec3(ls.x, ls.y, ls.z);
        vec3  w0 = cs * vec3(p0);
        vec3  w1 = cs * vec3(p1);
        float d2 = distance2(w0, w1);
        float r  = 1.0 / (k2 * attMin);
        if (d2 <= r)
        {
            float d   = sqrt(d2);
            float att = 1.0 / (k0 + k1 * d + k2 * d * d);
            if (att > attMin)
            {
                vec3  e = texelFetch(emission, p1, 0).rgb;
                float s = length(e);
                float v = vis(density, p1, p0, e, s);
                if (v > 0.0)
                {
                    float a = v * v * att;
                    l      += a * e;
                    i      += a * (w1 - w0);
                    p.x     = max(p.x, lp.x * a * s);
                }
            }
        }
        // Pulse freq of closest pulsing emitter
        if (lp.y > 0.0 && d2 < dmin)
        {
            p.y  = lp.y;
            dmin = d2;
        }
        // Keep count of overlapping frequencies
        if (d2 <= 1.33 * r && lp.y > 0.0 && lp.y != p.w)
        {
            p.w = lp.y;
            p.z += 1.0;
        }
    }
    // Normalize pulse depth and suppress overlapping frequencies
    p.x = p.z > 1.0 ? 0.0 : length(l) > 0.0 ? p.x / length(l) : 0.0;

    // Horizon
    int circ     = (size.x - 1) * 2 + (size.y - 1) * 2;
    int edge0    = size.x - 1;
    int edge1    = edge0 + size.y - 1;
    int edge2    = edge1 + size.y - 1;
    float weight = 1.0 / HORZ_SAMPLES;

    for (int x = 0; x < HORZ_SAMPLES; ++x)
    {
        vec2 uv = vec2(x / float(HORZ_SAMPLES),
                       1.0 - p0.z / float(size.z));

        int c   = int(circ * uv.x);
        ivec2 p = c <= edge0 ? ivec2(c, 0) :
                  c <= edge1 ? ivec2(0, c - edge0) :
                  c <= edge2 ? ivec2(c - edge1, size.y - 1) :
                  ivec2(size.x - 1, c - edge2);

        ivec3 p1 = ivec3(p, uv.y * (size.z - 1) + 0.5);
        vec4  h  = texture(horizon, uv);
        if (h.a > 0.0)
        {
            vec3 e  = h.rgb;
            float v = vis(density, p1, p0, e, length(e));
            if (v > 0.0)
            {
                l += weight * v * e;
                i += weight * v * v * normalize(vec3(p1) - vec3(p0));
            }
        }
    }

    light     = vec4(l, clamp(5.0 * p.x, 0.0, 1.0));
    incidence = vec4(i, 5.f * p.y);
}
