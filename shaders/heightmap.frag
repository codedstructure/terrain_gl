// terrain_gl
// @codedstructure 2023

#version 330 core
uniform float u_time;
uniform vec3 u_background;

in vec4 groundColour;
in vec3 groundNormal;
in vec2 groundPos;
out vec4 fragColor;
void main()
{

    float dist = gl_FragCoord.z / gl_FragCoord.w;
    float fog = clamp(0.01 * dist, 0., 1.);
    const float fogStart = 0.75;
    if (fog > fogStart) {
        fog = mix(0., 1., ((fog - fogStart)/(1.-fogStart)));
    } else {
        fog = 0;
    }
    vec4 fogColour = vec4(u_background.rgb, 1.);
    if (false) { // }(int(groundPos.x * 10)) % 10 == 0 || (int(groundPos.y * 10) % 10 == 0)){
        fragColor = fogColour;
    } else {
        fragColor = mix(
            vec4(max(0.1,
                        dot(groundNormal, normalize(vec3(1., 1, 1.)))
                ) * groundColour.rgb, 1.),
            fogColour,
            fog);
    }
}
