#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "objfile.h"

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

void mtllib_read(char* obj_path, char* filename, array* mtls) {
  FILE* fp;
  char path[1000];
  char line[1000];
  char* next;
  char* command;
  material m;

  snprintf(path, 1000, "%s/%s", dirname(obj_path), filename);
  fp = fopen(path, "rt");

  m.name = NULL;
  while (fgets(line, 99, fp)) {
    next = line;
    command = strsep(&next, " \t\r\n");
    switch (command[0] + command[1]*256) {
      case 'n' + 'e'*256:  // "newmtl"
        if (m.name) array_append(mtls, &m);
        m.name = strdup(strsep(&next, " \t\r\n"));
        m.ka.r = m.ka.g = m.ka.b = 0;
        m.kd.r = m.kd.g = m.kd.b = 0;
        m.ks.r = m.ks.g = m.ks.b = 0;
        m.alpha = 1;
        m.ns = 0;
        m.map_ka = NULL;
        m.map_kd = NULL;
        m.map_ks = NULL;
        break;
      case 'K' + 'a'*256:  // "Ka"
        sscanf(next, "%lg %lg %lg", &m.ka.r, &m.ka.g, &m.ka.b);
        break;
      case 'K' + 'd'*256:  // "Kd"
        sscanf(next, "%lg %lg %lg", &m.kd.r, &m.kd.g, &m.kd.b);
        break;
      case 'K' + 's'*256:  // "Ks"
        sscanf(next, "%lg %lg %lg", &m.ks.r, &m.ks.g, &m.ks.b);
        break;
      case 'd':  // "d"
      case 'T' + 'r'*256:  // "Tr"
        sscanf(next, "%lg", &m.alpha);
        break;
      case 'N' + 's'*256:  // "Ns"
        sscanf(next, "%lg", &m.ns);
        break;
      case 'm' + 'a'*256:  // "map_..."
        if (strcmp(command, "map_Ka") == 0) {
          m.map_ka = strdup(strsep(&next, " \t\r\n"));
        }
        if (strcmp(command, "map_Kd") == 0) {
          m.map_kd = strdup(strsep(&next, " \t\r\n"));
        }
        if (strcmp(command, "map_Ks") == 0) {
          m.map_ks = strdup(strsep(&next, " \t\r\n"));
        }
        break;
    }
  }
  if (m.name) array_append(mtls, &m);
  fclose(fp);
}

void mtllib_free(array* mtls) {
  int mi;
  material* m;
  for (mi = 0, m = mtls->items; mi < mtls->count; mi++, m++) {
    free(m->name);
    if (m->map_ka) free(m->map_ka);
    if (m->map_kd) free(m->map_kd);
    if (m->map_ks) free(m->map_ks);
  }
}

obj* obj_read(char* path) {
  FILE* fp;
  char line[1000];
  char* next;
  char* command;
  char* arg;
  int i, mi, vi, vti, vni;
  vector v, vt, vn, w;
  double x, y, z, len;
  face f;
  face* ff;
  material* m = NULL;
  obj* o;

  fp = fopen(path, "rt");
  if (!fp) return NULL;

  o = malloc(sizeof(obj));
  o->mtllib = NULL;
  o->mtls = array_new(sizeof(material));
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
        f.m = m;
        array_append(o->fs, &f);
        break;
      case 'm' + 't'*256:  // "mtllib"
        o->mtllib = strdup(strsep(&next, " \t\r\n"));
        mtllib_read(path, o->mtllib, o->mtls);
        break;
      case 'g':  // "g"
      case 'u' + 's'*256:  // "usemtl"
        arg = strsep(&next, " \t\r\n");
        for (mi = 0, m = o->mtls->items; mi < o->mtls->count; mi++, m++) {
          if (strcmp(arg, m->name) == 0) {
            break;
          }
        }
        if (mi >= o->mtls->count) {
          m = NULL;
        }
        break;
    }
  }
  fclose(fp);

  // Compute a default normal for every face.
  for (i = 0, ff = o->fs->items; i < o->fs->count; i++, ff++) {
    v.x = ff->vs[1].v->x - ff->vs[0].v->x;
    v.y = ff->vs[1].v->y - ff->vs[0].v->y;
    v.z = ff->vs[1].v->z - ff->vs[0].v->z;
    w.x = ff->vs[2].v->x - ff->vs[0].v->x;
    w.y = ff->vs[2].v->y - ff->vs[0].v->y;
    w.z = ff->vs[2].v->z - ff->vs[0].v->z;
    x = v.y*w.z - w.y*v.z;
    y = v.z*w.x - w.z*v.x;
    z = v.x*w.y - w.x*v.y;
    len = sqrt(x*x + y*y + z*z);
    ff->n.x = x/len;
    ff->n.y = y/len;
    ff->n.z = z/len;
  }
  return o;
}

void obj_write(obj* o, char* path) {
  FILE* fp;
  vector* vits;
  face* fits;
  vertex* v;
  material* m = NULL;
  int i, j;

  fp = fopen(path, "wt");
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
    if (fits[i].m != m) {
      m = fits[i].m;
      fprintf(fp, "usemtl %s\ng %s\n", m->name, m->name);
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
  fclose(fp);
}

void obj_free(obj* o) {
  int i;

  if (o->mtllib) free(o->mtllib);
  mtllib_free(o->mtls);
  array_free(o->mtls);
  array_free(o->vs);
  array_free(o->vts);
  array_free(o->vns);
  array_free(o->fs);
  free(o);
}
