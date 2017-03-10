#version 150

float vis(sampler3D tex, ivec3 p0, ivec3 p1, inout vec3 e, float el)
{
    // Make results symmetrical between endpoints
    if (p0.y > p1.y || p0.z > p1.z)
    {
        ivec3 t = p0;
        p0 = p1;
        p1 = t;
    }

    ivec3 d = p1 - p0;
    int   n = abs(d.x) > abs(d.y) ? abs(d.x) : abs(d.y);
    vec3  s = vec3(d) / n;

    float v = 1.0;
    vec3 p  = vec3(p0) + s + 0.5;

    for (int i = 0; i < n - 1 && v > 0.0; ++i, p += s)
    {
        vec4 d  = texelFetch(tex, ivec3(p), 0);
        float a = abs(d.a);
        e = d.a < 0 ? mix(e, el * d.rgb, min(1.0, 1.0 - a)) : e;
        v -= a;
    }
    return max(0.0, v);
}

float visSym(sampler3D tex, vec3 p0, vec3 p1, inout vec3 e, float el)
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
            vec4 d  = texelFetch(tex, ivec3(p), 0);
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
