#ifndef _cube_h_
#define _cube_h_

typedef struct {
    unsigned int xyz;
    unsigned int uvts;
} VertexData;

void make_cube_face_greedy(
    VertexData *data, float ao[4], float light[4],
    int face_dir, int w,
    float x, float y, float z, float n,
    float x_length, float y_length, float z_length);

void make_cube_faces(
    VertexData *data, float ao[6][4], float light[6][4],
    int left, int right, int top, int bottom, int front, int back,
    int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
    float x, float y, float z, float n);

void make_cube(
    VertexData *data, float ao[6][4], float light[6][4],
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n, int w);

void make_plant(
    VertexData *data, float ao, float light,
    float px, float py, float pz, float n, int w, float rotation);

void make_player(
    VertexData *data,
    float x, float y, float z, float rx, float ry);

void make_cube_wireframe(
    float *data, float x, float y, float z, float n);

void make_character(
    float *data,
    float x, float y, float n, float m, char c);

void make_character_3d(
    float *data, float x, float y, float z, float n, int face, char c);

void make_sphere(float *data, float r, int detail);

#endif
