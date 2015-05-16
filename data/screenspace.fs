#version 150
#extension GL_ARB_separate_shader_objects : enable

uniform vec4 color;
in vec3      dist;

layout (location = 0)
in float vertexAo;

layout (location = 0)
out vec4 colorF;

void main()
{
    vec3 n     = normalize(vec3(dFdx(gl_FragCoord.w), dFdy(gl_FragCoord.w), 1));
    vec3 l     = normalize(vec3(1, 10, 0));

    float d    = min(min(dist[0], dist[1]), dist[2]);
    float edge = exp2(-0.25 * d * d);

    vec4 color = vec4(vertexAo * color.xyz + 1000 * dot(n, l), 1);
    colorF     = mix(color, color + 0.5, edge);
}
