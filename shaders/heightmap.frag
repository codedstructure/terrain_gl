// terrain_gl
// @codedstructure 2023

#version 410 core
uniform float u_time;
uniform vec3 u_background;

in vec4 groundColour;
in vec3 groundNormal;
in vec2 groundPos;
out vec4 fragColor;
void main()
{
    float fog = clamp(0.001 * gl_FragCoord.z / gl_FragCoord.w, 0., 1.);
    vec4 fogColour = vec4(u_background.rgb, 1.0);
    if ((int(groundPos.x * 10)) % 10 == 0 || (int(groundPos.y * 10) % 10 == 0)){
        fragColor = fogColour;
    } else {
        fragColor = mix(vec4(max(0.4,
            3. * dot(
                groundNormal, normalize(vec3(1., 1., 1.))
            )
        ) * groundColour.rgb, 1.0), fogColour, fog);
        //fragColor = vec4(groundNormal.x, groundNormal.y, groundNormal.z, 1.0);
    }
}
