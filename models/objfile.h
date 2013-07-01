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
  vertex vs[3];
  vector n;
  char* m;
} face;

typedef struct {
  char* mtllib;
  array* mtls;
  array* vs;
  array* vts;
  array* vns;
  array* fs;
} obj;

obj* obj_read(FILE* fp);
void obj_free(obj* o);
