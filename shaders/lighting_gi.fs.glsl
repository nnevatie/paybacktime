#version 150

// Uniforms
uniform sampler2D texDepth;
uniform sampler3D texGi;
uniform mat4      w;
uniform vec3      boundsMin;
uniform vec3      boundsSize;

// Const
vec3 sizeTexGi = textureSize(texGi, 0);

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

// Externals
vec4 textureTricubic(sampler3D, vec3, vec3);
vec3 world(sampler2D depth, vec2 uv, mat4 w);
vec3 worldUvw(vec3 pos, vec3 boundsMin, vec3 boundsMax);

void main(void)
{
    vec3 worldPos = world(texDepth, ib.uv, w);
    vec3 uvwGi    = worldUvw(worldPos, boundsMin, boundsSize);
    vec3 gi       = texture(texGi, uvwGi).rgb;
    color         = vec4(gi, 1.0);
}
