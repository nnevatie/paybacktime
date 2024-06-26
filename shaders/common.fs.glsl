#version 150

float cmin(vec3 v)
{
    return min(v.z, min(v.x, v.y));
}

float cmax(vec3 v)
{
    return max(v.z, max(v.x, v.y));
}

float linearDepth(float depth, float zNear, float zFar)
{
    return (2 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

float linearDepth(float depth, mat4 proj)
{
  const float drmin = 0.0;
  const float drmax = 1.0;
  float ndcZ = (2.0 * depth - drmin - drmax) / (drmax - drmin);
  return proj[3][2] / ((proj[2][3] * ndcZ) - proj[2][2]);
}

vec3 world(sampler2D depth, vec2 uv, mat4 w)
{
    vec4 clip  = vec4(uv * 2.0 - 1.0, texture(depth, uv).r * 2.0 - 1.0, 1.0);
    vec4 world = w * clip;
    return world.xyz / world.w;
}

vec3 worldUvw(vec3 pos, vec3 boundsMin, vec3 boundsSize)
{
    return ((pos - boundsMin) / (boundsSize)).xzy;
}

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

vec4 textureBicubic(sampler3D sampler, vec3 uv)
{
    vec2 ts      = textureSize(sampler, 0).xy;
    vec2 tsInv   = 1.0 / ts;
    uv.xy       *= ts - 0.0; // 0.5?
    vec2 fxy     = fract(uv.xy);
    uv.xy       -= fxy;
    vec4 xcubic  = cubic(fxy.x);
    vec4 ycubic  = cubic(fxy.y);
    vec4 c       = uv.xxyy + vec2(-0.5, +1.5).xyxy;
    vec4 s       = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset  = c + vec4(xcubic.yw, ycubic.yw) / s;
    offset      *= tsInv.xxyy;
    vec4 sample0 = texture(sampler, vec3(offset.xz, uv.z));
    vec4 sample1 = texture(sampler, vec3(offset.yz, uv.z));
    vec4 sample2 = texture(sampler, vec3(offset.xw, uv.z));
    vec4 sample3 = texture(sampler, vec3(offset.yw, uv.z));
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
            vec4 s  = texture(sampler, uv1 + vec2(txs.x * float(m),
                                                    txs.y * float(n)));
            vec2 f  = vec2(bell(float(m) - ab.x), bell(float(n) - ab.y));
            vec4 c1 = vec4(f.x);
            vec4 c2 = vec4(f.y);
            sum    += s * c1 * c2;
            den    += c1 * c2;
        }

    return sum / den;
}

vec4 textureTricubic(sampler3D tex, vec3 uvw, vec3 texSize)
{
    vec3 uvwg = uvw * texSize - 0.5;
    vec3 i    = floor(uvwg);
    vec3 f0   = uvwg - i;
    vec3 f1   = 1.0 - f0;

    vec3 w0 = 1.0 / 6.0 * f1 * f1 *f1;
    vec3 w1 = 2.0 / 3.0 - 0.5 * f0 * f0 * (2.0 - f0);
    vec3 w2 = 2.0 / 3.0 - 0.5 * f1 * f1 * (2.0 - f1);
    vec3 w3 = 1.0 / 6.0 * f0 * f0 *f0;

    vec3 g0 = w0 + w1;
    vec3 g1 = w2 + w3;
    vec3 m  = 1.0 / texSize;
    vec3 h0 = m * ((w1 / g0) - 0.5 + i);
    vec3 h1 = m * ((w3 / g1) + 1.5 + i);

    vec4 tex000 = texture(tex, h0);
    vec4 tex100 = texture(tex, vec3(h1.x, h0.y, h0.z));
    tex000 = mix(tex100, tex000, g0.x);
    vec4 tex010 = texture(tex, vec3(h0.x, h1.y, h0.z));
    vec4 tex110 = texture(tex, vec3(h1.x, h1.y, h0.z));
    tex010 = mix(tex110, tex010, g0.x);
    tex000 = mix(tex010, tex000, g0.y);
    vec4 tex001 = texture(tex, vec3(h0.x, h0.y, h1.z));
    vec4 tex101 = texture(tex, vec3(h1.x, h0.y, h1.z));
    tex001 = mix(tex101, tex001, g0.x);
    vec4 tex011 = texture(tex, vec3(h0.x, h1.y, h1.z));
    vec4 tex111 = texture(tex, h1);
    tex011 = mix(tex111, tex011, g0.x);
    tex001 = mix(tex011, tex001, g0.y);
    return mix(tex001, tex000, g0.z);
}

float g1v(float dotNV, float k)
{
    return 1.0f / (dotNV * (1.0f - k) + k);
}

float ggx(vec3 N, vec3 V, vec3 L, float roughness, float F0)
{
  float alpha    = roughness * roughness;
  vec3 H         = normalize(L - V);
  float dotLH    = max(0.0, dot(L, H));
  float dotNH    = max(0.0, dot(N, H));
  float dotNL    = max(0.0, dot(N, L));
  float alphaSqr = alpha * alpha;
  float denom    = dotNH * dotNH * (alphaSqr - 1.0) + 1.0;
  float D        = alphaSqr / (3.141592653589793 * denom * denom);
  float F        = F0 + (1.0 - F0) * pow(1.0 - dotLH, 5.0);
  float k        = 0.5 * alpha;
  float k2       = k * k;
  return dotNL * D * F / (dotLH * dotLH * (1.0 - k2) + k2);
}
