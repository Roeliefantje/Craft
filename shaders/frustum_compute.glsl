#version 430 core

#define CHUNK_SIZE  32
#define RENDER_CHUNK_RADIUS  64
#define MIN_Y 0
#define MAX_Y 256

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(std430, binding = 0) buffer InputBuffer {
    float data[];
};

layout(std430, binding = 1) buffer OutputBuffer {
    int result[];
};

// int x = p * CHUNK_SIZE - 1;
//     int z = q * CHUNK_SIZE - 1;
//     int d = CHUNK_SIZE + 1;
//     float points[8][3] = {
//         {x + 0, miny, z + 0},
//         {x + d, miny, z + 0},
//         {x + 0, miny, z + d},
//         {x + d, miny, z + d},
//         {x + 0, maxy, z + 0},
//         {x + d, maxy, z + 0},
//         {x + 0, maxy, z + d},
//         {x + d, maxy, z + d}
//     };
//     int n = g->ortho ? 4 : 6;
//     for (int i = 0; i < n; i++) {
//         int in = 0;
//         int out = 0;
//         for (int j = 0; j < 8; j++) {
//             float d =
//                 planes[i][0] * points[j][0] +
//                 planes[i][1] * points[j][1] +
//                 planes[i][2] * points[j][2] +
//                 planes[i][3];
//             if (d < 0) {
//                 out++;
//             }
//             else {
//                 in++;
//                 break;
//             }
//         }
//         if (in == 0) {
//             return 0;
//         }
//     }
//     return 1;

void main() {
    if(gl_GlobalInvocationID.x >= RENDER_CHUNK_RADIUS * 2 || gl_GlobalInvocationID.y >= RENDER_CHUNK_RADIUS * 2)
        return;

    uint gid = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * RENDER_CHUNK_RADIUS * 2;

    float planes[6][4];
    for (int p = 0; p < 6; p++){
        int offset = p * 4;
        planes[p][0] = data[offset]; 
        planes[p][1] = data[offset + 1]; 
        planes[p][2] = data[offset + 2]; 
        planes[p][3] = data[offset + 3]; 
    }

    float x = (float(gl_GlobalInvocationID.x) - RENDER_CHUNK_RADIUS) * CHUNK_SIZE - 1 ;
    float z = (float(gl_GlobalInvocationID.y) - RENDER_CHUNK_RADIUS) * CHUNK_SIZE - 1;
    float d = CHUNK_SIZE + 1;
    float points[8][3] = {
        {x + 0, MIN_Y, z + 0},
        {x + d, MIN_Y, z + 0},
        {x + 0, MIN_Y, z + d},
        {x + d, MIN_Y, z + d},
        {x + 0, MAX_Y, z + 0},
        {x + d, MAX_Y, z + 0},
        {x + 0, MAX_Y, z + d},
        {x + d, MAX_Y, z + d}
    };

    int visible = 1;
    for (int i = 0; i < 6; i++){
        int inside = 0;
        for (int j = 0; j < 8; j++) {
            float d =
                planes[i][0] * points[j][0] +
                planes[i][1] * points[j][1] +
                planes[i][2] * points[j][2] +
                planes[i][3];
            if (d >= 0) {
                inside = 1;
                break;
            }
        }

        if(inside == 0){
            visible = 0;
            break;
        }

    }

    result[gid] = visible;
}