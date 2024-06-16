#include <math.h>
#include "cube.h"
#include "item.h"
#include "matrix.h"
#include "util.h"

const float normalized_light_direction[3] = { -0.577350f, 0.577350f, -0.577350f };

void make_cube_face_greedy(
    VertexData *data, float ao[4], float light[4],
    int face_dir, int w,
    float x, float y, float z, float n,
    float x_length, float y_length, float z_length)
{
    static const float positions[6][3] = { // int faces[6] = {left, right, top, bottom, front, back};
        {-1,-1,-1},
        {+1,-1, -1},
        {-1, +1, -1},
        {-1, -1, -1},
        {-1, -1, -1},
        {-1, -1, +1},
    };
    static const float offsets[6][6][3] = {
        {{1, 0, 0}, {0, 0, 1}, {1, 0, 1},{+1, -1, +1}, {+1, +1, -1}, {+1, +1, +1}},
        {{+1, -1, +1}, {+1, +1, -1}, {+1, +1, +1},{+1, -1, +1}, {+1, +1, -1}, {+1, +1, +1}},
        {{1, 0, 0}, {1, 0, 1}, {0, 0, 0}, {0, 0, 0}, {1, 0, 1}, {0, 0, 1}},
        {{-1, -1, +1}, {+1, -1, -1}, {+1, -1, +1},{+1, -1, +1}, {+1, +1, -1}, {+1, +1, +1}},
        {{-1, +1, -1}, {+1, -1, -1}, {+1, +1, -1},{+1, -1, +1}, {+1, +1, -1}, {+1, +1, +1}},
        {{-1, +1, +1}, {+1, -1, +1}, {+1, +1, +1},{+1, -1, +1}, {+1, +1, -1}, {+1, +1, +1}}
    };
    static const float normals[6][3] = {
        {-1, 0, 0},
        {+1, 0, 0},
        {0, +1, 0},
        {0, -1, 0},
        {0, 0, -1},
        {0, 0, +1}
    };

    static const unsigned int normal_flags[6] = {
        0, 1, 2, 3, 4, 5,
    };


    static const float uvs[6][4][2] = {
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
        {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
        {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{1, 0}, {1, 1}, {0, 0}, {0, 1}}
    };
    static const float indices[6][6] = {
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3}
    };
    static const float flipped[6][6] = {
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1},
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1},
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1}
    };
    //float *d = data;
    VertexData *vdp = (VertexData*)data;
    float s = 0.0625;
    float a = 0 + 1 / 2048.0;
    float b = s - 1 / 2048.0;
    // int faces[6] = {left, right, top, bottom, front, back};
    // int tiles[6] = {wleft, wright, wtop, wbottom, wfront, wback};
    float du = (w % 16) * s;
    float dv = (w / 16) * s;
    int flip = ao[0] + ao[3] > ao[1] + ao[2];
    for (int v = 5; v >= 0; v--) {
        int j = flip ? flipped[face_dir][v] : indices[face_dir][v];
        VertexData vd;

        float x_modulo = fmod(x, 32);
        x_modulo < 1 ? x_modulo += 32 : x_modulo;
        float z_modulo = fmod(z, 32);
        z_modulo < 1 ? z_modulo += 32 : z_modulo;

        float pos[3] ={
            ((n * positions[face_dir][0]) + (offsets[face_dir][v][0] * x_length - n)),
            ((n * positions[face_dir][1]) + (offsets[face_dir][v][1] * y_length - n)),
            ((n * positions[face_dir][2]) + (offsets[face_dir][v][2] * z_length - n)),
        };
        unsigned int xi = x_modulo + pos[0] + 0.5;
        unsigned int yi = y + pos[1] + 0.5;
        unsigned int zi = z_modulo + pos[2] + 0.5;


        vd.xyz = ((xi & 0xFF) << 24) | ((yi & 0xFF) << 16) | ((zi & 0xFF) << 8);


        vd.diffuse_bake =  normals[face_dir][0] * normalized_light_direction[0] + 
        normals[face_dir][1] * normalized_light_direction[1] + 
        normals[face_dir][2] * normalized_light_direction[2];

        vd.u = du + (uvs[face_dir][j][0] ? b : a);
        vd.v = dv + (uvs[face_dir][j][1] ? b : a);
        vd.t = ao[j];
        vd.s = light[j];
        *(vdp++) = vd;
    }
}

void make_cube_faces_new(
    VertexData *data, float ao[6][4], float light[6][4],
    int left, int right, int top, int bottom, int front, int back,
    int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
    float x, float y, float z, float n)
{
    static const float positions[6][4][3] = {
        {{-1, -1, -1}, {-1, -1, +1}, {-1, +1, -1}, {-1, +1, +1}},
        {{+1, -1, -1}, {+1, -1, +1}, {+1, +1, -1}, {+1, +1, +1}},
        {{-1, +1, -1}, {-1, +1, +1}, {+1, +1, -1}, {+1, +1, +1}},
        {{-1, -1, -1}, {-1, -1, +1}, {+1, -1, -1}, {+1, -1, +1}},
        {{-1, -1, -1}, {-1, +1, -1}, {+1, -1, -1}, {+1, +1, -1}},
        {{-1, -1, +1}, {-1, +1, +1}, {+1, -1, +1}, {+1, +1, +1}}
    };
    static const float normals[6][3] = {
        {-1, 0, 0},
        {+1, 0, 0},
        {0, +1, 0},
        {0, -1, 0},
        {0, 0, -1},
        {0, 0, +1}
    };

    static const unsigned int normal_flags[6] = {
        0, 1, 2, 3, 4, 5,
    };


    static const float uvs[6][4][2] = {
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
        {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
        {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{1, 0}, {1, 1}, {0, 0}, {0, 1}}
    };
    static const float indices[6][6] = {
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3}
    };
    static const float flipped[6][6] = {
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1},
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1},
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1}
    };
    //float *d = data;
    VertexData *vdp = (VertexData*)data;
    float s = 0.0625;
    float a = 0 + 1 / 2048.0;
    float b = s - 1 / 2048.0;
    int faces[6] = {left, right, top, bottom, front, back};
    int tiles[6] = {wleft, wright, wtop, wbottom, wfront, wback};
    for (int i = 0; i < 6; i++) {
        if (faces[i] == 0) {
            continue;
        }
        float du = (tiles[i] % 16) * s;
        float dv = (tiles[i] / 16) * s;
        int flip = ao[i][0] + ao[i][3] > ao[i][1] + ao[i][2];
        for (int v = 0; v < 6; v++) {
            int j = flip ? flipped[i][v] : indices[i][v];
            VertexData vd;
            //print the x value
            // vd.x = fmod(x + n * positions[i][j][0], 32);
            // vd.x = fabs(fmodf(x, 32)) + n * positions[i][j][0];
            float x_modulo = fmod(x, 32);
            x_modulo < 0 ? x_modulo += 32 : x_modulo;
            float z_modulo = fmod(z, 32);
            z_modulo < 0 ? z_modulo += 32 : z_modulo;

            

            // vd.x = x_modulo + n * positions[i][j][0];
            // vd.y = y + n * positions[i][j][1];
            // // vd.z = fabs(fmodf(z, 32)) + n * positions[i][j][2];
            // vd.z = z_modulo + n * positions[i][j][2];
            
            unsigned int xi = x_modulo + n * positions[i][j][0] + 0.5;
            unsigned int yi = y + n * positions[i][j][1] + 0.5;
            unsigned int zi = z_modulo + n * positions[i][j][2] + 0.5;

            // printf("X value: %d\n", x);
            // printf("Y value: %d\n", y);
            // printf("Float value y: %f\n", vd.y);
            // printf("Z value: %d\n", z);
            //print X value converted to int:


            vd.xyz = ((xi & 0xFF) << 24) | ((yi & 0xFF) << 16) | ((zi & 0xFF) << 8);

            //printf("xyz: %u\n", vd.xyz); 

            vd.diffuse_bake =  normals[i][0] * normalized_light_direction[0] + 
            normals[i][1] * normalized_light_direction[1] + 
            normals[i][2] * normalized_light_direction[2];
            // vd.nx = normals[i][0];
            // vd.ny = normals[i][1];
            // vd.nz = normals[i][2];
            vd.u = du + (uvs[i][j][0] ? b : a);
            vd.v = dv + (uvs[i][j][1] ? b : a);
            vd.t = ao[i][j];
            vd.s = light[i][j];
            *(vdp++) = vd;
        }
        // for (int v = 0; v < 6; v++) {
        //     int j = flip ? flipped[i][v] : indices[i][v];
        //     *(d++) = x + n * positions[i][j][0];
        //     *(d++) = y + n * positions[i][j][1];
        //     *(d++) = z + n * positions[i][j][2];
        //     //*(unsigned char*)(d++) = normal_flags[i]; 
        //     *(d++) = normals[i][0];
        //     *(d++) = normals[i][1];
        //     *(d++) = normals[i][2];
        //     *(d++) = du + (uvs[i][j][0] ? b : a);
        //     *(d++) = dv + (uvs[i][j][1] ? b : a);
        //     *(d++) = ao[i][j];
        //     *(d++) = light[i][j];
        // }
    }
}

void make_cube_new(
    VertexData *data, float ao[6][4], float light[6][4],
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n, int w)
{
    int wleft = blocks[w][0];
    int wright = blocks[w][1];
    int wtop = blocks[w][2];
    int wbottom = blocks[w][3];
    int wfront = blocks[w][4];
    int wback = blocks[w][5];
    make_cube_faces_new(
        data, ao, light,
        left, right, top, bottom, front, back,
        wleft, wright, wtop, wbottom, wfront, wback,
        x, y, z, n);
}

void make_plant_new(
    VertexData *data, float ao, float light,
    float px, float py, float pz, float n, int w, float rotation)
{
    //printf("Making plant");
    static const float positions[4][4][3] = {
        {{ 0, -1, -1}, { 0, -1, +1}, { 0, +1, -1}, { 0, +1, +1}},
        {{ 0, -1, -1}, { 0, -1, +1}, { 0, +1, -1}, { 0, +1, +1}},
        {{-1, -1,  0}, {-1, +1,  0}, {+1, -1,  0}, {+1, +1,  0}},
        {{-1, -1,  0}, {-1, +1,  0}, {+1, -1,  0}, {+1, +1,  0}}
    };
    static const float normals[4][3] = {
        {-1, 0, 0},
        {+1, 0, 0},
        {0, 0, -1},
        {0, 0, +1}
    };

    static const unsigned int normal_flags[4] = {
        6, 7, 8, 9,
    };

    static const float uvs[4][4][2] = {
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
        {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{1, 0}, {1, 1}, {0, 0}, {0, 1}}
    };
    static const float indices[4][6] = {
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3}
    };
    //float *d = data;
    VertexData *vdp = data;
    float s = 0.0625;
    float a = 0;
    float b = s;
    float du = (plants[w] % 16) * s;
    float dv = (plants[w] / 16) * s;
    // printf("Start of new plant\n");
    for (int i = 0; i < 4; i++) {
        // printf("Start of new face\n");
        for (int v = 0; v < 6; v++) {
            int j = indices[i][v];
            VertexData vd;

            float px_m = fmod( px, 32);
            px_m < 0 ? px_m += 32 : px_m;
            float pz_m = fmod( pz, 32);
            pz_m < 0 ? pz_m += 32 : pz_m;
            

            // printf("Float px: %f\n", px_m + n * positions[i][j][1]);
            // printf("Float py: %f\n", py + n * positions[i][j][1]);
            // printf("Float pz: %f\n", pz_m + n * positions[i][j][2]);
            int pxi = px_m + n * positions[i][j][0] + 0.5;
            int pyi = py + n * positions[i][j][1] + 0.5;
            int pzi = pz_m + n * positions[i][j][2] + 0.5;
            // printf("int px: %d\n", pxi);
            // printf("int py: %d\n", pyi);
            // printf("int pz: %d\n", pzi);

            vd.xyz = ((pxi & 0xFF) << 24) | ((pyi & 0xFF) << 16) | ((pzi & 0xFF) << 8);

            // vd.x = n * positions[i][j][1];
            // vd.y = n * positions[i][j][1];
            // vd.z = n * positions[i][j][2];

            float nm[3] = {normals[i][0], normals[i][1],normals[i][2]};
            // rotate_y(nm, RADIANS(rotation));
            vd.diffuse_bake =  nm[0] * normalized_light_direction[0] + 
            nm[1] * normalized_light_direction[1] + 
            nm[2] * normalized_light_direction[2];
            // vd.nx = normals[i][0];
            // vd.ny = normals[i][1];
            // vd.nz = normals[i][2];
            vd.u = du + (uvs[i][j][0] ? b : a);
            vd.v = dv + (uvs[i][j][1] ? b : a);
            vd.t = ao;
            vd.s = light;
            *(vdp++) = vd;



            // *(d++) = n * positions[i][j][0];
            // *(d++) = n * positions[i][j][1];
            // *(d++) = n * positions[i][j][2];
            // *(d++) = normals[i][0];
            // *(d++) = normals[i][1];
            // *(d++) = normals[i][2];
            // *(d++) = du + (uvs[i][j][0] ? b : a);
            // *(d++) = dv + (uvs[i][j][1] ? b : a);
            // *(d++) = ao;
            // *(d++) = light;
        }
    }
    float ma[16];
    float mb[16];
    mat_identity(ma);
    mat_rotate(mb, 0, 1, 0, RADIANS(rotation));
    mat_multiply(ma, mb, ma);
    //mat_apply(data, ma, 24, 3, 8); //CHANGED FROM 10 TO 8, I LOVE RANDOM INTS...
    //mat_translate(mb, px, py, pz);
    mat_multiply(ma, mb, ma);
    //mat_apply(data, ma, 24, 0, 8); //CHANGED FROM 10 TO 8, I LOVE RANDOM INTS...
}



void make_player(
    float *data,
    float x, float y, float z, float rx, float ry)
{
    //printf("making player");
    float ao[6][4] = {0};
    float light[6][4] = {
        {0.8, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.8}
    };
    make_cube_faces_new(
        (VertexData*)data, ao, light,
        1, 1, 1, 1, 1, 1,
        226, 224, 241, 209, 225, 227,
        0, 0, 0, 0.4);
    float ma[16];
    float mb[16];
    mat_identity(ma);
    mat_rotate(mb, 0, 1, 0, rx);
    mat_multiply(ma, mb, ma);
    mat_rotate(mb, cosf(rx), 0, sinf(rx), -ry);
    mat_multiply(ma, mb, ma);
    //mat_apply(data, ma, 36, 3, 10);
    mat_translate(mb, x, y, z);
    mat_multiply(ma, mb, ma);
    mat_apply(data, ma, 36, 0, 10);
}

void make_cube_wireframe(float *data, float x, float y, float z, float n) {
    static const float positions[8][3] = {
        {-1, -1, -1},
        {-1, -1, +1},
        {-1, +1, -1},
        {-1, +1, +1},
        {+1, -1, -1},
        {+1, -1, +1},
        {+1, +1, -1},
        {+1, +1, +1}
    };
    static const int indices[24] = {
        0, 1, 0, 2, 0, 4, 1, 3,
        1, 5, 2, 3, 2, 6, 3, 7,
        4, 5, 4, 6, 5, 7, 6, 7
    };
    float *d = data;
    for (int i = 0; i < 24; i++) {
        int j = indices[i];
        *(d++) = x + n * positions[j][0];
        *(d++) = y + n * positions[j][1];
        *(d++) = z + n * positions[j][2];
    }
}

void make_character(
    float *data,
    float x, float y, float n, float m, char c)
{
    float *d = data;
    float s = 0.0625;
    float a = s;
    float b = s * 2;
    int w = c - 32;
    float du = (w % 16) * a;
    float dv = 1 - (w / 16) * b - b;
    *(d++) = x - n; *(d++) = y - m;
    *(d++) = du + 0; *(d++) = dv;
    *(d++) = x + n; *(d++) = y - m;
    *(d++) = du + a; *(d++) = dv;
    *(d++) = x + n; *(d++) = y + m;
    *(d++) = du + a; *(d++) = dv + b;
    *(d++) = x - n; *(d++) = y - m;
    *(d++) = du + 0; *(d++) = dv;
    *(d++) = x + n; *(d++) = y + m;
    *(d++) = du + a; *(d++) = dv + b;
    *(d++) = x - n; *(d++) = y + m;
    *(d++) = du + 0; *(d++) = dv + b;
}

void make_character_3d(
    float *data, float x, float y, float z, float n, int face, char c)
{
    static const float positions[8][6][3] = {
        {{0, -2, -1}, {0, +2, +1}, {0, +2, -1},
         {0, -2, -1}, {0, -2, +1}, {0, +2, +1}},
        {{0, -2, -1}, {0, +2, +1}, {0, -2, +1},
         {0, -2, -1}, {0, +2, -1}, {0, +2, +1}},
        {{-1, -2, 0}, {+1, +2, 0}, {+1, -2, 0},
         {-1, -2, 0}, {-1, +2, 0}, {+1, +2, 0}},
        {{-1, -2, 0}, {+1, -2, 0}, {+1, +2, 0},
         {-1, -2, 0}, {+1, +2, 0}, {-1, +2, 0}},
        {{-1, 0, +2}, {+1, 0, +2}, {+1, 0, -2},
         {-1, 0, +2}, {+1, 0, -2}, {-1, 0, -2}},
        {{-2, 0, +1}, {+2, 0, -1}, {-2, 0, -1},
         {-2, 0, +1}, {+2, 0, +1}, {+2, 0, -1}},
        {{+1, 0, +2}, {-1, 0, -2}, {-1, 0, +2},
         {+1, 0, +2}, {+1, 0, -2}, {-1, 0, -2}},
        {{+2, 0, -1}, {-2, 0, +1}, {+2, 0, +1},
         {+2, 0, -1}, {-2, 0, -1}, {-2, 0, +1}}
    };
    static const float uvs[8][6][2] = {
        {{0, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, 1}},
        {{1, 0}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {0, 1}},
        {{1, 0}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {0, 1}},
        {{0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1}},
        {{0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1}},
        {{0, 1}, {1, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}},
        {{0, 1}, {1, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}},
        {{0, 1}, {1, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}}
    };
    static const float offsets[8][3] = {
        {-1, 0, 0}, {+1, 0, 0}, {0, 0, -1}, {0, 0, +1},
        {0, +1, 0}, {0, +1, 0}, {0, +1, 0}, {0, +1, 0},
    };
    float *d = data;
    float s = 0.0625;
    float pu = s / 5;
    float pv = s / 2.5;
    float u1 = pu;
    float v1 = pv;
    float u2 = s - pu;
    float v2 = s * 2 - pv;
    float p = 0.5;
    int w = c - 32;
    float du = (w % 16) * s;
    float dv = 1 - (w / 16 + 1) * s * 2;
    x += p * offsets[face][0];
    y += p * offsets[face][1];
    z += p * offsets[face][2];
    for (int i = 0; i < 6; i++) {
        *(d++) = x + n * positions[face][i][0];
        *(d++) = y + n * positions[face][i][1];
        *(d++) = z + n * positions[face][i][2];
        *(d++) = du + (uvs[face][i][0] ? u2 : u1);
        *(d++) = dv + (uvs[face][i][1] ? v2 : v1);
    }
}

int _make_sphere(
    float *data, float r, int detail,
    float *a, float *b, float *c,
    float *ta, float *tb, float *tc)
{
    if (detail == 0) {
        float *d = data;
        *(d++) = a[0] * r; *(d++) = a[1] * r; *(d++) = a[2] * r;
        *(d++) = a[0]; *(d++) = a[1]; *(d++) = a[2];
        *(d++) = ta[0]; *(d++) = ta[1];
        *(d++) = b[0] * r; *(d++) = b[1] * r; *(d++) = b[2] * r;
        *(d++) = b[0]; *(d++) = b[1]; *(d++) = b[2];
        *(d++) = tb[0]; *(d++) = tb[1];
        *(d++) = c[0] * r; *(d++) = c[1] * r; *(d++) = c[2] * r;
        *(d++) = c[0]; *(d++) = c[1]; *(d++) = c[2];
        *(d++) = tc[0]; *(d++) = tc[1];
        return 1;
    }
    else {
        float ab[3], ac[3], bc[3];
        for (int i = 0; i < 3; i++) {
            ab[i] = (a[i] + b[i]) / 2;
            ac[i] = (a[i] + c[i]) / 2;
            bc[i] = (b[i] + c[i]) / 2;
        }
        normalize(ab + 0, ab + 1, ab + 2);
        normalize(ac + 0, ac + 1, ac + 2);
        normalize(bc + 0, bc + 1, bc + 2);
        float tab[2], tac[2], tbc[2];
        tab[0] = 0; tab[1] = 1 - acosf(ab[1]) / PI;
        tac[0] = 0; tac[1] = 1 - acosf(ac[1]) / PI;
        tbc[0] = 0; tbc[1] = 1 - acosf(bc[1]) / PI;
        int total = 0;
        int n;
        n = _make_sphere(data, r, detail - 1, a, ab, ac, ta, tab, tac);
        total += n; data += n * 24;
        n = _make_sphere(data, r, detail - 1, b, bc, ab, tb, tbc, tab);
        total += n; data += n * 24;
        n = _make_sphere(data, r, detail - 1, c, ac, bc, tc, tac, tbc);
        total += n; data += n * 24;
        n = _make_sphere(data, r, detail - 1, ab, bc, ac, tab, tbc, tac);
        total += n; data += n * 24;
        return total;
    }
}

void make_sphere(float *data, float r, int detail) {
    // detail, triangles, floats
    // 0, 8, 192
    // 1, 32, 768
    // 2, 128, 3072
    // 3, 512, 12288
    // 4, 2048, 49152
    // 5, 8192, 196608
    // 6, 32768, 786432
    // 7, 131072, 3145728
    static int indices[8][3] = {
        {4, 3, 0}, {1, 4, 0},
        {3, 4, 5}, {4, 1, 5},
        {0, 3, 2}, {0, 2, 1},
        {5, 2, 3}, {5, 1, 2}
    };
    static float positions[6][3] = {
        { 0, 0,-1}, { 1, 0, 0},
        { 0,-1, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0, 0, 1}
    };
    static float uvs[6][3] = {
        {0, 0.5}, {0, 0.5},
        {0, 0}, {0, 0.5},
        {0, 1}, {0, 0.5}
    };
    int total = 0;
    for (int i = 0; i < 8; i++) {
        int n = _make_sphere(
            data, r, detail,
            positions[indices[i][0]],
            positions[indices[i][1]],
            positions[indices[i][2]],
            uvs[indices[i][0]],
            uvs[indices[i][1]],
            uvs[indices[i][2]]);
        total += n; data += n * 24;
    }
}
