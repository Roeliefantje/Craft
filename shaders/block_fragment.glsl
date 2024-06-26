#version 120

uniform sampler2D sampler;
uniform sampler2D sky_sampler;
uniform float timer;
uniform float daylight;
uniform int ortho;

varying vec2 fragment_uv;
varying float fragment_ao;
varying float fragment_light;
varying float fog_factor;
varying float fog_height;
varying float diffuse;
varying vec4 local_position;
varying vec2 uv_scalar;

const float pi = 3.14159265;
const float tile_size = 0.0625;

void main() {
    vec2 uv_offset = floor(uv_scalar);
    vec2 uv = fragment_uv - uv_offset * tile_size;
    vec3 color = vec3(texture2D(sampler, uv));
    if (color == vec3(1.0, 0.0, 1.0)) {
        discard;
    }
    bool cloud = color == vec3(1.0, 1.0, 1.0);
    if (cloud && bool(ortho)) {
        discard;
    }
    float df = cloud ? 1.0 - diffuse * 0.2 : diffuse;
    float ao = cloud ? 1.0 - (1.0 - fragment_ao) * 0.2 : fragment_ao;
    ao = min(1.0, ao + fragment_light);
    df = min(1.0, df + fragment_light);
    float value = min(1.0, daylight + fragment_light);
    vec3 light_color = vec3(value * 0.3 + 0.2);
    vec3 ambient = vec3(value * 0.3 + 0.2);
    vec3 light = ambient + light_color * df;
    color = clamp(color * light * ao, vec3(0.0), vec3(1.0));
    vec3 sky_color = vec3(texture2D(sky_sampler, vec2(timer, fog_height)));
    color = mix(color, sky_color, fog_factor);
    

    gl_FragColor = vec4(color, 1.0);
    // gl_FragColor = vec4(local_position.y / 255, local_position.y / 255, local_position.y / 255, 1);
    // gl_FragColor = vec4(local_position.x / 255, local_position.y / 255, local_position.z / 255, 1);
}
