// terrain_gl
// @codedstructure 2023

#version 330 core
uniform float u_time;
uniform vec3 u_background;
uniform sampler2D u_texture;
uniform float value_a;
uniform float value_b;

in vec4 groundColour;
in vec3 groundNormal;
in vec2 groundPos;
in float groundHeight;
out vec4 fragColor;

void main()
{

    float dist = gl_FragCoord.z / gl_FragCoord.w;
    float fog = 0; // clamp(0.003 * dist, 0., 1.);
    const float fogStart = 0.95;
    if (fog > fogStart) {
        fog = mix(0., 1., ((fog - fogStart)/(1.-fogStart)));
    } else {
        fog = 0;
    }
    vec4 fogColour = vec4(u_background.rgb, 1.);

    float diffuse = dot(normalize(groundNormal), normalize(vec3(0.5, 1, 0.5)));

    float isoline = sin(groundHeight)/2. + 0.5;
    isoline = pow(isoline, 200.);

    isoline = 0;

    if (groundHeight > 0) {
        fragColor = texture(u_texture, groundPos.xy / 35.) * 0.5;
        fragColor += groundColour * 0.5;
        fragColor += vec4(vec3(isoline), 1) * 0.5;
    } else {
        fragColor = vec4(0, 0.1, 0.3, 1.);
    }
    fragColor = mix(
        vec4((0.2 + diffuse * 0.8) * fragColor.rgb, 1.),
        fogColour,
        fog);

    // DEBUG NORMAL SHADER
    /// fragColor = vec4(groundNormal * 0.5 + vec3(0.5), 1);
}
