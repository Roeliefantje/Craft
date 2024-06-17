#version 330 core

uniform mat4 matrix;
uniform vec3 camera;
uniform float fog_distance;
uniform int ortho;
uniform vec2 chunk_pos;
uniform int chunk_size;

in vec4 position;

in float diffuse_bake;

//in vec3 normal;
in vec4 uv;
in uint position_uint;
in uint uvts;
in uint uvScales;

out vec2 fragment_uv;
out float fragment_ao;
out float fragment_light;
out float fog_factor;
out float fog_height;
out float diffuse;
out vec2 uv_scalar;
out vec4 local_position;

const float tile_size = 0.0625;
const float pi = 3.14159265;
const vec3 light_direction = normalize(vec3(-1.0, 1.0, -1.0));
const vec3 normals[10] = vec3[](vec3(-1, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, -1, 0), vec3(0, 0, -1), vec3(0, 0, 1), //Cube
normalize(vec3(1,0,-1)), normalize(vec3(-1,0,1)), normalize(vec3(-1,0,-1)), normalize(vec3(1,0,1))); //Plants

vec4 getPosition(uint pos) {
    float x = int(pos >> 24);
    float y = int(pos >> 16) & 0xFF;
    float z = int(pos >> 8) & 0xFF;
    return vec4(x, y, z, 1);
}

vec3 getNormal(uint pos) {
    int flag = int(pos) & 0xFF;
    return normals[flag];
}

bool isPlant(uint pos) {
    int flag = int(pos) & 0xFF;
    if (flag > 5) {
        return true;
    } else {
        return false;
    }

}

vec2 getUV(uint uvts) {
    int u = int(uvts >> 24);
    int v = int(uvts >> 16) & 0xFF;

    return vec2(u, v) * tile_size;

}

float getAO(uint uvts) {
    int ao = int(uvts >> 8) & 0xFF;
    
    return 0.3 + (1.0 - ao / 2.0) * 0.7;
}

float getLight(uint uvts) {
    int light = int(uvts) & 0xFF;
    return light / 16.0;
}

vec2 getUVScalar(uint uvScales){
    return vec2(int(uvScales >> 24), int(uvScales >> 16)& 0xFF);
}

void main() {
    //Local_position is passed to the fragment shader for debugging.
    local_position = getPosition(position_uint);
    // -0.5 to align back to original, doesnt really do anything but still
    vec4 converted_position = (getPosition(position_uint) + vec4(chunk_pos.x * chunk_size, 0.0, chunk_pos.y * chunk_size, 0.0)) - vec4(0.5, 0.5, 0.5, 0);

    if (isPlant(position_uint)) {
        int normal_flag = int(position_uint) & 0xFF;
        if (normal_flag == 8 ) { //Correct
            converted_position.z += 0.5;
        } else if (normal_flag == 7 || normal_flag == 6) { //Correct
            converted_position.x += 0.5;
        } else {
            converted_position.z += 0.5;
        }

        //TODO: Add rotation based on which vertex we are
    }


    gl_Position = matrix * converted_position;

    uv_scalar = getUVScalar(uvScales);

    fragment_uv = getUV(uvts);
    // fragment_uv = uv.xy;
    fragment_ao = getAO(uvts);
    //fragment_ao = 0.3 + (1.0 - uv.z) * 0.7;
    // fragment_light = uv.w;
    fragment_light = getLight(uvts);
    vec3 normal = getNormal(position_uint);
    diffuse = max(0.0, dot(normal, light_direction));
    if (ortho != 0) {
        fog_factor = 0.0;
        fog_height = 0.0;
    }
    else {
        float camera_distance = distance(camera, vec3(converted_position));
        fog_factor = pow(clamp(camera_distance / fog_distance, 0.0, 1.0), 4.0);
        float dy = converted_position.y - camera.y;
        float dx = distance(converted_position.xz, camera.xz);
        fog_height = (atan(dy, dx) + pi / 2) / pi;
    }
}
