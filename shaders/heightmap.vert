// terrain_gl
// @codedstructure 2023

#version 330 core
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

    // Derive normal using a Sobel filter
    float topLeft = textureOffset(u_heightmap, tpos, ivec2(-1.0, 1.)).r;
    float topMiddle = textureOffset(u_heightmap, tpos, ivec2(0., 1.)).r;
    float topRight = textureOffset(u_heightmap, tpos, ivec2(1.0, 1.)).r;
    float left = textureOffset(u_heightmap, tpos, ivec2(-1., 0.)).r;
    float right = textureOffset(u_heightmap, tpos, ivec2(1., 0.)).r;
    float bottomLeft = textureOffset(u_heightmap, tpos, ivec2(-1.0, -1.)).r;
    float bottomMiddle = textureOffset(u_heightmap, tpos, ivec2(0., -1.)).r;
    float bottomRight = textureOffset(u_heightmap, tpos, ivec2(1.0, -1.)).r;
    // (-1,-2,-1, 0,0,0, 1,2,1) for both left->right and top->bottom
    float x = topRight + right * 2. + bottomRight - topLeft - left * 2. - bottomLeft;
    float z = bottomLeft + bottomMiddle * 2. + bottomRight - topLeft - topMiddle * 2. - topRight;
    groundNormal = normalize(vec3(x, 1., z));

    float height = texture(u_heightmap, tpos).r - 1.;
    gl_Position = u_mvpMatrix * vec4(world_pos.x, height, world_pos.y, 1.);
    groundPos = vPos.xz;
    groundColour = vec4(height, 0.5, 0.5, 1.);
}
