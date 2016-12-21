#version 150

// Uniforms
uniform sampler3D      density;
uniform sampler3D      emission;
uniform isamplerBuffer lightSrc;
uniform int            wz;
uniform int            lsc;
uniform float          attMin;
uniform float          k0;
uniform float          k1;
uniform float          k2;
uniform vec3           cs;

// Const
ivec3 size = ivec3(textureSize(density, 0));

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Outputs
out vec3 light;
out vec3 incidence;

void swap(inout ivec3 v0, inout ivec3 v1)
{
    ivec3 t = v0;
    v0 = v1;
    v1 = t;
}

float vis(ivec3 p0, ivec3 p1)
{
    // Grid deltas and step
    ivec3 d  = p1 - p0;
    ivec3 da = abs(d);
    ivec3 sg = sign(d);

    // t-step, t-max
    vec3 st = 1.0 / da;
    int   n = da.x + da.y + da.z;

    // Loop
    vec3  m = st;
    ivec3 p = p0;
    float v = 1.0;
    for (int i = 0; i < n && v > 0.0; ++i)
    {
        if (i > 0)
            v -= texelFetch(density, p, 0).r;

        if (m.x < m.y && m.x < m.z)
        {
            p.x += sg.x;
            m.x += st.x;
        }
        else
        if (m.y < m.z)
        {
            p.y += sg.y;
            m.y += st.y;
        }
        else
        {
            p.z += sg.z;
            m.z += st.z;
        }
    }
    return v;
}

float distance2(vec3 v0, vec3 v1)
{
    vec3 d = v0 - v1;
    return d.x * d.x + d.y * d.y + d.z * d.z;
}

void main(void)
{
    ivec3 p0 = ivec3(gl_FragCoord.xy, wz);
    vec3 l   = vec3(0);
    vec3 i   = vec3(0);

    for (int li = 0; li < lsc; ++li)
    {
        ivec4 ls = texelFetch(lightSrc, li);
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
                float v = vis(p1, p0);
                if (v > 0.0)
                {
                    vec3  e = texelFetch(emission, p1, 0).rgb;
                    float a = v * v * att;
                    l      += a * e;
                    i      += a * (w1 - w0);
                }
            }
        }
    }

    light     = l;
    incidence = i;
}
