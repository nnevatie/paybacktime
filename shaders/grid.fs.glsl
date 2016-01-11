#version 150

// Const
const float PI = 3.14159265358979323846;

// Uniforms
uniform mat4      v;
uniform vec3      pos;
uniform vec4      gridColor;

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
    float frq = PI / 8.0;
    return color * smoothstep(.98, 1.2,
           max(1.05 * sin((p.x + 4.0) * frq), 1.05 * sin((p.y + 4.0) * frq)));
}

void main(void)
{
    vec3  r = normalize(ib.viewRay) * mat3(v);
    float d = intersect(pos, r);
    color   = max(0, 1 - d * 0.0015) * grid((pos + d * r).xz, gridColor);
}
