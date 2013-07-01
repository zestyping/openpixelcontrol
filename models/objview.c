/* Copyright 2013 Google Inc.

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef __APPLE__
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "objfile.h"

// Camera parameters
#define FOV_DEGREES 20
int orbiting = 0, dollying = 0;
double start_angle, start_elevation, start_distance;
int start_x, start_y;
double orbit_angle = 192.0;  // camera orbit angle, degrees
double camera_elevation = -15;  // camera elevation angle, degrees
double camera_distance = 16.0;  // distance from origin, metres
double camera_aspect = 1.0;  // will be updated to match window aspect ratio

// Vector arithmetic
vector tmp_vector;
#define set_rgb(r, g, b) (glColor3d(r, g, b))
#define put_vertex(v) ((tmp_vector = v), glVertex3dv(&(tmp_vector.x)))
#define put_pair(v, w) (put_vertex(v), put_vertex(w))

vector add(vector v, vector w) {
  vector result;
  result.x = v.x + w.x;
  result.y = v.y + w.y;
  result.z = v.z + w.z;
  return result;
}

vector subtract(vector v, vector w) {
  vector result;
  result.x = v.x - w.x;
  result.y = v.y - w.y;
  result.z = v.z - w.z;
  return result;
}

vector multiply(double f, vector v) {
  vector result;
  result.x = f*v.x;
  result.y = f*v.y;
  result.z = f*v.z;
  return result;
}

double length(vector v) {
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

double dot(vector v, vector w) {
  return v.x*w.x + v.y*w.y + v.z*w.z;
}

vector cross(vector v, vector w) {
  vector result;
  result.x = v.y*w.z - w.y*v.z;
  result.y = v.z*w.x - w.z*v.x;
  result.z = v.x*w.y - w.x*v.y;
  return result;
}

// Drawing
void draw_axes() {
  vector o = {0, 0, 0};
  vector x = {1, 0, 0};
  vector y = {0, 1, 0};
  vector z = {0, 0, 1};
  vector xx = {10, 0, 0};
  vector yy = {0, 10, 0};
  vector zz = {0, 0, 10};

  glDisable(GL_LIGHTING);
  glLineWidth(2);
  glBegin(GL_LINES);
  set_rgb(0.3, 0.3, 0.3);
  put_pair(o, x);
  put_pair(o, y);
  put_pair(o, z);
  set_rgb(0.3, 0, 0);
  put_pair(x, xx);
  set_rgb(0, 0.3, 0);
  put_pair(y, yy);
  set_rgb(0, 0, 0.3);
  put_pair(z, zz);
  glEnd();
}

void draw_grid() {
  int i, j;
  double v;

  glDisable(GL_LIGHTING);
  glBegin(GL_POINTS);
  for (i = -5; i <= 5; i++) {
    for (j = -5; j <= 5; j++) {
      v = 0.3 + (i + j)*0.02;
      glColor3d(v, v, v);
      glVertex3d(i, j, 0);
    }
  }
  glEnd();
}

void compile_obj(obj* o, int list) {
  int fi;
  face* f;

  glNewList(list, GL_COMPILE);
  glBegin(GL_TRIANGLES);
  glColor3d(0.3, 0.3, 0.3);
  for (fi = 0, f = o->fs->items; fi < o->fs->count; fi++, f++) {
    glNormal3dv((double*) &(f->n));
    glVertex3dv((double*) f->vs[0].v);
    glVertex3dv((double*) f->vs[1].v);
    glVertex3dv((double*) f->vs[2].v);
  }
  glEnd();
  glEndList();
}

int obj_list = 0;

void draw_compiled_obj(obj* o) {
  if (!obj_list) {
    obj_list = 1;
    compile_obj(o, obj_list);
  }
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glCallList(obj_list);
}

void draw_obj(obj* o) {
  int fi;
  face* f;

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glBegin(GL_TRIANGLES);
  glColor3d(0.3, 0.3, 0.3);
  for (fi = 0, f = o->fs->items; fi < o->fs->count; fi++, f++) {
    glNormal3dv((double*) &(f->n));
    glVertex3dv((double*) f->vs[0].v);
    glVertex3dv((double*) f->vs[1].v);
    glVertex3dv((double*) f->vs[2].v);
  }
  glEnd();
}

obj* o = NULL;

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  draw_grid();
  draw_axes();
  //draw_obj(o);
  draw_compiled_obj(o);
  glutSwapBuffers();
}


// Interaction
void update_camera() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV_DEGREES, camera_aspect, 0.1, 1e3); // fov, aspect, zrange
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  double camera_y = -cos(camera_elevation*M_PI/180)*camera_distance;
  double camera_z = sin(camera_elevation*M_PI/180)*camera_distance;
  gluLookAt(0, camera_y, camera_z, /* target */ 0, 0, 0, /* up */ 0, 0, 1);
  glRotatef(orbit_angle, 0, 0, 1);
  display();
}

void reshape(int width, int height) {
  glViewport(0, 0, width, height);
  camera_aspect = ((double) width)/((double) height);
  update_camera();
}

void mouse(int button, int state, int x, int y) {
  if (state == GLUT_DOWN && glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
    dollying = 1;
    start_distance = camera_distance;
    start_x = x;
    start_y = y;
  } else if (state == GLUT_DOWN) {
    orbiting = 1;
    start_angle = orbit_angle;
    start_elevation = camera_elevation;
    start_x = x;
    start_y = y;
  } else {
    orbiting = 0;
    dollying = 0;
  }
}

void motion(int x, int y) {
  if (orbiting) {
    orbit_angle = start_angle + (x - start_x)*1.0;
    double elevation = start_elevation + (y - start_y)*1.0;
    camera_elevation = elevation < -89 ? -89 : elevation > 89 ? 89 : elevation;
    update_camera();
  }
  if (dollying) {
    double distance = start_distance + (y - start_y)*0.1;
    camera_distance = distance < 1.0 ? 1.0 : distance;
    update_camera();
  }
}

void keyboard(unsigned char key, int x, int y) {
  if (key == '\x1b' || key == 'q') exit(0);
}

int main(int argc, char** argv) {
  FILE* fp;

  glutInit(&argc, argv);
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <options> <filename.obj>\n", argv[0]);
    exit(1);
  }
  fprintf(stderr, "Loading %s...", argv[1]);
  fp = fopen(argv[1], "rt");
  o = obj_read(fp);
  fclose(fp);
  fprintf(stderr, " v: %d, f: %d\n", o->vs->count, o->fs->count);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(800, 600);
  glutCreateWindow(argv[1]);
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutIgnoreKeyRepeat(1);
  glutKeyboardFunc(keyboard);

  glEnable(GL_DEPTH_TEST);
#ifdef __APPLE__
  /* Make glutSwapBuffers wait for vertical refresh to avoid frame tearing. */
  int swap_interval = 1;
  CGLContextObj context = CGLGetCurrentContext();
  CGLSetParameter(context, kCGLCPSwapInterval, &swap_interval);
#endif

  glutMainLoop();
  obj_free(o);
  return 0;
}
