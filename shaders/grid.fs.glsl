#version 150

// Const
const float PI = 3.14159265358979323846;

// Uniforms
uniform sampler2D texDepth;
uniform vec3      pos;
uniform float     yaw;
uniform float     pitch;
uniform vec4      albedo;

// Input
in Block
{
    vec2 uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

float intersect(vec3 origin, vec3 dir)
{
    return length(dir * origin.y / dir.y);
}

vec4 grid(vec2 p, vec4 color)
{
    float frq = 0.25;
    return color * smoothstep(.98, 1.05,
           max(sin((p.x) * frq), sin((p.y) * frq)));
}

mat3 mx(float beta)
{
    return mat3(1, 0, 0,
                0,  cos(beta), sin(beta),
                0, -sin(beta), cos(beta));
}

mat3 my(float beta)
{
    return mat3(cos(beta), 0, sin(beta),
                0, 1, 0,
               -sin(beta), 0, cos(beta));
}

void main(void)
{
    if (texture(texDepth, ib.uv).r < 1.0)
        discard;

    vec3  r = normalize(ib.viewRay);
    vec3  p = pos;
    r      *= mx(-pitch) * my(yaw - PI / 2);
    float d = intersect(p, r);
    vec4  t = grid((p + d * r).xz, albedo);
    color   = max(0, 1 - d * 0.0015) * t;
}
