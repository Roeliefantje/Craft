#version 330 core

uniform mat4 matrix;
uniform vec3 camera;
uniform float fog_distance;
uniform int ortho;

in vec4 position;
in float diffuse_bake;

//in vec3 normal;
in vec4 uv;

out vec2 fragment_uv;
out float fragment_ao;
out float fragment_light;
out float fog_factor;
out float fog_height;
out float diffuse;

const float pi = 3.14159265;
const vec3 light_direction = normalize(vec3(-1.0, 1.0, -1.0));
const vec3 normals[10] = vec3[](vec3(-1, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, -1, 0), vec3(0, 0, -1), vec3(0, 0, 1), //Cube
normalize(vec3(1,0,-1)), normalize(vec3(-1,0,1)), normalize(vec3(-1,0,-1)), normalize(vec3(1,0,1))); //Plants

vec3 decodeNormal(uint flag) {
    return normals[flag];
    // if (flag == 0u) return vec3(-1, 0, 0);
    // else if (flag == 1u) return vec3(1, 0, 0);
    // else if (flag == 2u) return vec3(0, 1, 0);
    // else if (flag == 3u) return vec3(0, -1, 0);
    // else if (flag == 4u) return vec3(0, 0, -1);
    // else if (flag == 5u) return vec3(0, 0, 1);
    // else return vec3(0, 0, 0); // Should not happen
}

void main() {
    gl_Position = matrix * position;
    //vec3 normal = decodeNormal(normal_flag);
    fragment_uv = uv.xy;
    fragment_ao = 0.3 + (1.0 - uv.z) * 0.7;
    fragment_light = uv.w;
    //diffuse = max(0.0, dot(normal, light_direction));
    diffuse = diffuse_bake;
    if (ortho != 0) {
        fog_factor = 0.0;
        fog_height = 0.0;
    }
    else {
        float camera_distance = distance(camera, vec3(position));
        fog_factor = pow(clamp(camera_distance / fog_distance, 0.0, 1.0), 4.0);
        float dy = position.y - camera.y;
        float dx = distance(position.xz, camera.xz);
        fog_height = (atan(dy, dx) + pi / 2) / pi;
    }
}
