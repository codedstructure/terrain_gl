// terrain_gl
// @codedstructure 2023

#version 330 core
uniform float u_time;
uniform mat4 u_mvpMatrix;
uniform int u_layer;
uniform sampler2DArray u_heightmap;
uniform float u_grid_scale;
uniform float u_grid_size;
uniform float u_value_a;
uniform float u_value_b;
uniform vec2 u_grid_offset;
uniform int u_level_factor;
in vec3 vPos;
out vec4 groundColour;
out vec3 groundNormal;
out vec2 groundPos;
out float groundHeight;
out vec3 worldPos;

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

    x /= u_level_factor * u_grid_scale / u_grid_size;
    z /= u_level_factor * u_grid_scale / u_grid_size;

    return normalize(vec3(x, 1., z));
}

void main()
{
    vec2 world_pos = (u_grid_scale * u_grid_offset) + vPos.xz;
    vec2 patchpos = vPos.xz / u_grid_scale;

    float height = 0.0;
    // only use height for 'interior', i.e. not the skirts
    if (vPos.y >= 0.0) {
        // |0|1|2|3|4|5|6|7|
        // first bar is 0.0 last is 1.0
        // to get values exact need halfway between
        float num_pixels = u_grid_size * 5.0/4.0 + 1.0;
        float edge = 0.5 + u_grid_size/8.0;
        // 0 -> 1.5
        // 1 -> num_pixels-1.5
        vec3 tpos = vec3((num_pixels-edge*2.0)/num_pixels * patchpos / u_level_factor + vec2(edge/num_pixels), u_layer);

        float normal_offset = 1 / num_pixels;

        groundNormal = sobol(tpos, normal_offset);
        height = texture(u_heightmap, tpos).r;
        if (height < 1) {
            // height is max 1, so this results in 0..1
            float depth = min(4, -height + 1) / 4;
            depth = smoothstep(0, 1, depth);
            float waveTime = u_time;
            vec3 waterNormal1 = sobol(tpos, normal_offset / 4) * 10;
            float waterValue = sin(world_pos.x / 17 + waveTime * depth) +
                            cos(world_pos.x / 127 + waveTime / 7)  *
                            cos(world_pos.y / 137 + waveTime / 19) ;
            vec3 waterNormal2 = normalize(vec3(waterValue, 1, waterValue));
            groundNormal = 0.2 * waterNormal1 + 0.4 * waterNormal2;
            groundNormal = normalize(groundNormal + vec3(0, 1, 0));

            height = max(0, height);
        }
    }
    worldPos = vec3(world_pos.x, height, world_pos.y);
    gl_Position = u_mvpMatrix * vec4(worldPos, 1.);
    groundPos = vPos.xz;
    groundColour = vec4(0.5, 0.3, 0.2, 1.);
    groundHeight = height;
}
