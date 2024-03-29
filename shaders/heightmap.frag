// terrain_gl
// @codedstructure 2023

#version 330 core
uniform float u_time;
uniform vec3 u_background;
uniform sampler2D u_grass_tex;
uniform sampler2D u_stone_tex;
uniform float u_grid_scale;
uniform float u_value_a;
uniform float u_value_b;
uniform vec3 u_viewpos;

in vec4 groundColour;
in vec3 groundNormal;
in vec2 groundPos;
in float groundHeight;
in vec3 worldPos;
out vec4 fragColor;

void main()
{

    float dist = gl_FragCoord.z / gl_FragCoord.w;
    float fog = 0; // smoothstep(5000, 10000, dist);
    vec4 fogColour = vec4(u_background.rgb, 1.);

    float isoline = sin(groundHeight)/2. + 0.5;
    isoline = pow(isoline, 200.);

    isoline = 0;  // disable isoline

    vec4 waterColourNear = vec4(0.2, 0.7, 0.8, 1);
    vec4 waterColourFar = vec4(0.3, 0.5, 0.6, 1);
    vec4 waterColour = mix(waterColourNear, waterColourFar, clamp(0, 1, dist / 1000));

    vec4 hillColour = groundColour * 0.25;
    vec4 snow = texture(u_stone_tex, worldPos.xz / 95.) / 2 + vec4(0.5,0.5,0.5,1);
    vec4 stone = texture(u_stone_tex, worldPos.xz / 55.) / 2;
    vec4 grass = texture(u_grass_tex, worldPos.xz / 35.) / 2;
    if (groundHeight > 120) {
        hillColour += snow;
    } else if (groundHeight > 90) {
        hillColour += mix(stone, snow, (groundHeight - 90) / 30);
    } else if (groundHeight > 60) {
        hillColour += stone;
    } else if (groundHeight > 30) {
        hillColour += mix(grass, stone, (groundHeight - 30) / 30);
    } else {
        hillColour += grass;
    }

    hillColour += vec4(vec3(isoline), 1) * 0.25;
    float luminance = 0.3 * hillColour.r + 0.4 * hillColour.g + 0.3 * hillColour.b;
    if (dist > 4000) {
        hillColour.rgb = mix(hillColour.rgb, vec3(luminance), clamp(0, 1, (dist - 4000) / 10000));
    }

    float k_a, k_d, k_s;

    vec3 v_n = groundNormal;
    if (groundHeight < 3 + sin(u_time / 20)) {
        fragColor = mix(waterColour, hillColour, groundHeight / 4);
        k_s = 0.5;
        k_d = 0.2;
        k_a = 0.3;
    } else {
        fragColor = hillColour;
        v_n += (texture(u_stone_tex, (worldPos.xz + vec2(0.1)) / 100).rgb - vec3(0.5)) * 0.125;
        k_s = 0.0;
        k_d = 0.5;
        k_a = 0.7;
    }
    vec3 v_l = normalize(vec3(1, 0.5, 0.5));
    vec3 v_r = reflect(v_l, v_n);
    vec3 v_v = normalize(worldPos - u_viewpos);
    float specular = pow(max(0, dot(v_r, v_v)), 30);
    v_n = normalize(v_n);
    float diffuse = dot(v_n, v_l);
    fragColor = vec4((k_a + diffuse * k_d + specular * k_s) * fragColor.rgb, 1.);

    fragColor = mix(
        fragColor,
        fogColour,
        fog);

    // DEBUG NORMAL SHADER
    /// fragColor = vec4(groundNormal * 0.5 + vec3(0.5), 1);
}
