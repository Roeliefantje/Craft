#version 430 core

layout(local_size_x = 1) in;

layout(std430, binding = 0) buffer InputBuffer {
    float data[];
};

layout(std430, binding = 1) buffer OutputBuffer {
    int result[];
};

void main() {
    uint gid = gl_GlobalInvocationID.x;
    result[gid] = 255;
}