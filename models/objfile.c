#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  int capacity;
  int count;
  int size;
  void* items;
} array;

typedef struct {
  int index;
  double x, y, z;
} vector;

typedef struct {
  vector* v;
  vector* vt;
  vector* vn;
} vertex;

typedef struct {
  vertex vs[3];
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

array* array_new(int size) {
  array* a = malloc(sizeof(array));
  a->capacity = a->count = 0;
  a->items = NULL;
  a->size = size;
  return a;
}

void array_append(array* a, void* item) {
  if (a->count >= a->capacity) {
    a->capacity = a->capacity < 16 ? 16 : a->capacity*2;
    a->items = realloc(a->items, a->capacity*a->size);
  }
  memcpy(a->items + a->size*a->count++, item, a->size);
}

void array_free(array* a) {
  free(a->items);
  free(a);
}

obj* obj_read(FILE* fp) {
  char line[100];
  char* next;
  char* command;
  char* arg;
  char* mtl = NULL;
  int i, mi, vi, vti, vni;
  vector v, vt, vn;
  face f;
  obj* o = malloc(sizeof(obj));

  o->mtllib = NULL;
  o->mtls = array_new(sizeof(char*));
  o->vs = array_new(sizeof(vector));
  o->vts = array_new(sizeof(vector));
  o->vns = array_new(sizeof(vector));
  o->fs = array_new(sizeof(face));

  while (fgets(line, 99, fp)) {
    next = line;
    command = strsep(&next, " \t\r\n");
    switch (command[0] + command[1]*256) {
      case 'v':  // "v"
        sscanf(next, "%lg %lg %lg", &v.x, &v.y, &v.z);
        v.index = o->vs->count + 1;
        array_append(o->vs, &v);
        break;
      case 'v' + 't'*256:  // "vt"
        sscanf(next, "%lg %lg %lg", &vt.x, &vt.y, &vt.z);
        vt.index = o->vts->count + 1;
        array_append(o->vts, &vt);
        break;
      case 'v' + 'n'*256:  // "vn"
        sscanf(next, "%lg %lg %lg", &vn.x, &vn.y, &vn.z);
        vn.index = o->vns->count + 1;
        array_append(o->vns, &vn);
        break;
      case 'f':  // "f"
        for (i = 0; i < 3; i++) {
          arg = strsep(&next, " \t\r\n");
          vi = atoi(strsep(&arg, "/"));
          vti = arg ? atoi(strsep(&arg, "/")) : 0;
          vni = arg ? atoi(strsep(&arg, "/")) : 0;
          f.vs[i].v = vi ? (vector*) o->vs->items + (vi - 1) : NULL;
          f.vs[i].vt = vti ? (vector*) o->vts->items + (vti - 1) : NULL;
          f.vs[i].vn = vni ? (vector*) o->vns->items + (vni - 1) : NULL;
        }
        f.m = mtl;
        array_append(o->fs, &f);
        break;
      case 'm' + 't'*256:  // "mtllib"
        o->mtllib = strdup(strsep(&next, " \t\r\n"));
        break;
      case 'g':  // "g"
      case 'u' + 's'*256:  // "usemtl"
        arg = strsep(&next, " \t\r\n");
        for (mi = 0; mi < o->mtls->count; mi++) {
          mtl = ((char**) o->mtls->items)[mi];
          if (strcmp(arg, mtl) == 0) {
            break;
          }
        }
        if (mi >= o->mtls->count) {
          mtl = strdup(arg);
          array_append(o->mtls, &mtl);
        }
        break;
    }
  }
  return o;
}

void obj_write(obj* o, FILE* fp) {
  vector* vits;
  face* fits;
  vertex* v;
  char* mtl = NULL;
  int i, j;

  if (o->mtllib) fprintf(fp, "%s\n", o->mtllib);
  vits = (vector*) o->vs->items;
  for (i = 0; i < o->vs->count; i++) {
    fprintf(fp, "v %.12lg %.12lg %.12lg\n", vits[i].x, vits[i].y, vits[i].z);
    vits[i].index = i + 1;
  }
  vits = (vector*) o->vts->items;
  for (i = 0; i < o->vts->count; i++) {
    fprintf(fp, "vt %.12lg %.12lg %.12lg\n", vits[i].x, vits[i].y, vits[i].z);
    vits[i].index = i + 1;
  }
  vits = (vector*) o->vns->items;
  for (i = 0; i < o->vns->count; i++) {
    fprintf(fp, "vn %.12lg %.12lg %.12lg\n", vits[i].x, vits[i].y, vits[i].z);
    vits[i].index = i + 1;
  }
  fits = (face*) o->fs->items;
  for (i = 0; i < o->fs->count; i++) {
    if (fits[i].m != mtl) {
      mtl = fits[i].m;
      fprintf(fp, "usemtl %s\ng %s\n", mtl, mtl);
    }
    fprintf(fp, "f");
    for (j = 0; j < 3; j++) {
      v = &fits[i].vs[j];
      fprintf(fp, " %d", v->v->index);
      if (v->vt) {
        fprintf(fp, "/%d", v->vt->index);
      } else if (v->vn) {
        fprintf(fp, "/");
      }
      if (v->vn) {
        fprintf(fp, "/%d", v->vn->index);
      }
    }
    fprintf(fp, "\n");
  }
}

void obj_free(obj* o) {
  array_free(o->vs);
  array_free(o->vts);
  array_free(o->fs);
  free(o);
}
