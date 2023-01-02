// terrain_gl
// @codedstructure 2023

#version 410 core
uniform mat4 u_mvpMatrix;
uniform sampler2D u_heightmap;
uniform float u_grid_scale;
uniform vec2 u_grid_offset;
in vec3 vPos;
out vec4 groundColour;
out vec3 groundNormal;
out vec2 groundPos;

void main()
{
    vec2 world_pos = (u_grid_scale * u_grid_offset) + vPos.xz;
    vec2 tpos = vPos.xz / u_grid_scale;
    float hxl = textureOffset(u_heightmap, tpos, ivec2(-1, 0)).r;
    float hxr = textureOffset(u_heightmap, tpos, ivec2(1, 0)).r;
    float hyl = textureOffset(u_heightmap, tpos, ivec2(0, -1)).r;
    float hyr = textureOffset(u_heightmap, tpos, ivec2(0, 1)).r;
    vec3 u = normalize(vec3(0.01, 0., hxr-hxl));
    vec3 v = normalize(vec3(0., 0.01, hyr-hyl));
    groundNormal = cross(u, v);

    float height = texture(u_heightmap, tpos).r - 1.;
    gl_Position = u_mvpMatrix * vec4(world_pos.x, height, world_pos.y, 1.);
    groundPos = vPos.xz;
    groundColour = vec4(height, 0., 0.5, 1.);
}
