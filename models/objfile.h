#include <stdio.h>

typedef struct {
  int capacity;
  int count;
  int size;
  void* items;
} array;

typedef struct {
  double x, y, z;
  int index;
} vector;

typedef struct {
  vector* v;
  vector* vt;
  vector* vn;
} vertex;

typedef struct {
  double r, g, b;
} color;

typedef struct {
  char* name;
  color ka, kd, ks;
  double alpha, ns;
  char* map_ka;
  char* map_kd;
  char* map_ks;
} material;

typedef struct {
  vertex vs[3];
  vector n;
  material* m;
} face;

typedef struct {
  char* mtllib;
  array* mtls;
  array* vs;
  array* vts;
  array* vns;
  array* fs;
} obj;

obj* obj_read(char* path);  // load .obj file, creating a new obj object
void obj_write(obj* o, char* path);  // write an obj (does not write mtllib)
void obj_free(obj* o);  // deallocates an obj object
