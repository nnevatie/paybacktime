#version 150

vec4 cubic(float v)
{
    vec4 n  = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s  = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0 / 6.0);
}

float bell(float x)
{
    float f = (x / 2.0) * 1.5;
    if(f > -1.5 && f < -0.5) return 0.5 * pow(f + 1.5, 2.0);
    else
    if(f > -0.5 && f < 0.5) return 3.0 / 4.0 - (f * f);
    else
    if(f > 0.5 && f < 1.5) return 0.5 * pow(f - 1.5, 2.0);
    return 0.0;
}

vec4 textureBicubic(sampler2D sampler, vec2 uv)
{
    vec2 ts      = textureSize(sampler, 0);
    vec2 tsInv   = 1.0 / ts;
    uv          *= ts - 0.5;
    vec2 fxy     = fract(uv);
    uv          -= fxy;
    vec4 xcubic  = cubic(fxy.x);
    vec4 ycubic  = cubic(fxy.y);
    vec4 c       = uv.xxyy + vec2(-0.5, +1.5).xyxy;
    vec4 s       = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset  = c + vec4(xcubic.yw, ycubic.yw) / s;
    offset      *= tsInv.xxyy;
    vec4 sample0 = texture(sampler, offset.xz);
    vec4 sample1 = texture(sampler, offset.yz);
    vec4 sample2 = texture(sampler, offset.xw);
    vec4 sample3 = texture(sampler, offset.yw);
    float sx     = s.x / (s.x + s.y);
    float sy     = s.z / (s.z + s.w);
    return mix(mix(sample3, sample2, sx), mix(sample1, sample0, sx), sy);
}

vec4 textureBicubicBell(sampler2D sampler, vec2 uv0)
{
    vec2 ts  = textureSize(sampler, 0);
    vec2 txs = vec2(1.0) / ts;
    float a  = fract(uv0.x * ts.x);
    float b  = fract(uv0.y * ts.y);
    vec2 ab  = fract(uv0 * ts);
    vec2 uv1 = vec2(int(uv0.x * ts.x), int(uv0.y * ts.y)) / ts + vec2(0.0) / ts;

    vec4 sum = vec4(0.0);
    vec4 den = vec4(0.0);
    for(int m = -1; m <= 2; ++m)
        for(int n = -1; n <= 2; ++n)
        {
            vec4 s  = texture2D(sampler, uv1 + vec2(txs.x * float(m),
                                                    txs.y * float(n)));
            vec2 f  = vec2(bell(float(m) - ab.x), bell(float(n) - ab.y));
            vec4 c1 = vec4(f.x);
            vec4 c2 = vec4(f.y);
            sum    += s * c1 * c2;
            den    += c1 * c2;
        }

    return sum / den;
}
