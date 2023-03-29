// terrain_gl
// @codedstructure 2023

#version 330 core
uniform mat4 u_mvpMatrix;
uniform int u_layer;
uniform sampler2DArray u_heightmap;
uniform float u_grid_scale;
uniform float value_a;
uniform float value_b;
uniform vec2 u_grid_offset;
uniform int u_level_factor;
in vec3 vPos;
out vec4 groundColour;
out vec3 groundNormal;
out vec2 groundPos;
out float groundHeight;

vec3 sobol(vec3 tpos, float offset)
{
    // Derive normal using a Sobel filter
    float topLeft = texture(u_heightmap, vec3(tpos.xy + offset * ivec2(-1., 1.),tpos.z)).r;
    float topMiddle = texture(u_heightmap, vec3(tpos.xy + offset * ivec2(0., 1.),tpos.z)).r;
    float topRight = texture(u_heightmap, vec3(tpos.xy + offset * ivec2(1., 1.),tpos.z)).r;
    float left = texture(u_heightmap, vec3(tpos.xy + offset * ivec2(-1., 0.),tpos.z)).r;
    float right = texture(u_heightmap, vec3(tpos.xy + offset * ivec2(1., 0.),tpos.z)).r;
    float bottomLeft = texture(u_heightmap, vec3(tpos.xy + offset * ivec2(-1., -1.),tpos.z)).r;
    float bottomMiddle = texture(u_heightmap, vec3(tpos.xy + offset * ivec2(0., -1.),tpos.z)).r;
    float bottomRight = texture(u_heightmap, vec3(tpos.xy + offset * ivec2(1., -1.),tpos.z)).r;
    // (-1,-2,-1, 0,0,0, 1,2,1) for both left->right and top->bottom
    float x = topRight + right * 2. + bottomRight - topLeft - left * 2. - bottomLeft;
    float z = bottomLeft + bottomMiddle * 2. + bottomRight - topLeft - topMiddle * 2. - topRight;
    return normalize(vec3(x, 1., z));
}

void main()
{
    vec2 world_pos = (u_grid_scale * u_grid_offset) + vPos.xz;
    // 0.8 below undoes the 1.25 texture scaling required to be able to
    // offset into it, avoiding edge effects when computing normals.
    vec2 patchpos = 0.8 * vPos.xz / u_grid_scale + vec2(0.125, 0.125);

    vec3 tpos = vec3(patchpos / u_level_factor, u_layer);

    groundNormal = sobol(tpos, 0.0001) / 2 + sobol(tpos, 0.0005);

    float height = texture(u_heightmap, tpos).r;
    if (height < 0) {
        height = 0;
        groundNormal = vec3(0, 1, 0);
    }
    gl_Position = u_mvpMatrix * vec4(world_pos.x, height, world_pos.y, 1.);
    groundPos = vPos.xz;
    groundColour = vec4(0.5, 0.3, 0.2, 1.);
    groundHeight = height;
}
