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

float vis(vec3 p0, vec3 p1, inout vec3 e, float el)
{
    p0 += 0.5; p1 += 0.5;

    // Grid deltas and step
    vec3 d  = p1 - p0;
    vec3 da = abs(d);
    vec3 sg = sign(d);

    // t-step, iteration count
    vec3 st = 1.0 / da;
    int   n = int(da.x + da.y + da.z);

    // Loop
    vec3  m = 0.5 * st;
    vec3  p = p0;
    float v = 1.0;

    for (int i = 0; i < n && v > 0.0; ++i)
    {
        if (i > 0)
        {
            vec4 d  = texelFetch(density, ivec3(p), 0);
            float a = abs(d.a);
            e = d.a < 0 ? mix(e, el * d.rgb, min(1.0, 1.0 - a)) : e;
            v -= a;
        }

        bvec3 lt  = lessThan(m.xxy, m.yzz);
        bvec3 lti = not(lt);

        if (lt.x && lt.y)
        {
            p.x += sg.x;
            m.x += st.x;
        }
        if (lti.x && lt.z)
        {
            p.y += sg.y;
            m.y += st.y;
        }
        if ((lt.x && lti.y) || (lti.x && lti.z))
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
                vec3  e = texelFetch(emission, p1, 0).rgb;
                float v = vis(p1, p0, e, length(e));
                if (v > 0.0)
                {
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
