/* Copyright 2013 Ka-Ping Yee

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

#include <libgen.h>
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
#include "soil/SOIL.h"
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
  glColor3d(0.3, 0.3, 0.3);
  glVertex3d(0, 0, 0);
  glVertex3d(1, 0, 0);
  glVertex3d(0, 0, 0);
  glVertex3d(0, 1, 0);
  glVertex3d(0, 0, 0);
  glVertex3d(0, 0, 1);
  glColor3d(0.3, 0, 0);
  glVertex3d(1, 0, 0);
  glVertex3d(10, 0, 0);
  glColor3d(0, 0.3, 0);
  glVertex3d(0, 1, 0);
  glVertex3d(0, 10, 0);
  glColor3d(0, 0, 0.3);
  glVertex3d(0, 0, 1);
  glVertex3d(0, 0, 10);
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

GLuint texs[32];
unsigned char* image;
int enable_textures = 1;

void load_texture(int index, char* filename) {
  GLuint tex;
  int width, height, x, y;
  unsigned char* image;
  unsigned char* a;
  unsigned char* b;
  unsigned char tmp;
  char command[1000];

  texs[index] = -1;
  image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGB);
  fprintf(stderr, "Texture %s: ", filename);
  if (!image && strstr(SOIL_last_result(), "progressive")) {
    snprintf(command, 1000, "jpegtran '%s' > %s", filename, "/tmp/tex.jpg");
    system(command);
    image = SOIL_load_image("/tmp/tex.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    unlink("/tmp/tex.jpg");
  }
  if (!image) {
    fprintf(stderr, "%s\n", SOIL_last_result());
    return;
  }
  fprintf(stderr, "%d x %d\n", width, height);

  // Flip the image vertically.
  for (y = 0; y*2 < height; y++) {
    a = image + y*width*3;
    b = image + (height - 1 - y)*width*3;
    for (x = 0; x < width*3; x++, a++, b++) {
      tmp = *a;
      *a = *b;
      *b = tmp;
    }
  }

  glGenTextures(1, &texs[index]);
  glBindTexture(GL_TEXTURE_2D, texs[index]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, image);
  SOIL_free_image_data(image);
}

void draw_obj(obj* o) {
  int fi;
  int mi;
  material* m;
  face* f;

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  for (mi = 0, m = o->mtls->items; mi < o->mtls->count; mi++, m++) {
    if (enable_textures && texs[mi] >= 0) {
      glEnable(GL_TEXTURE_2D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, texs[mi]);
    } else {
      glDisable(GL_TEXTURE_2D);
    }
    glColor3d(m->kd.r, m->kd.g, m->kd.b);

    glBegin(GL_TRIANGLES);
    for (fi = 0, f = o->fs->items; fi < o->fs->count; fi++, f++) {
      if (f->m == m) {
        glNormal3dv((double*) &(f->n));
        if (f->vs[0].vt) glTexCoord2dv((double*) f->vs[0].vt);
        glVertex3dv((double*) f->vs[0].v);
        if (f->vs[1].vt) glTexCoord2dv((double*) f->vs[1].vt);
        glVertex3dv((double*) f->vs[1].v);
        if (f->vs[2].vt) glTexCoord2dv((double*) f->vs[2].vt);
        glVertex3dv((double*) f->vs[2].v);
      }
    }
    glEnd();
  }
}

void draw_compiled_obj(obj* o) {
  static int list = 0;
  if (!list) {
    list = 1;
    glNewList(list, GL_COMPILE);
    draw_obj(o);
    glEndList();
  }
  glCallList(list);
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
  if (key == 't') enable_textures = !enable_textures;
}

int main(int argc, char** argv) {
  int mi;
  material* m;
  char path[1000];

  glutInit(&argc, argv);
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <options> <filename.obj>\n", argv[0]);
    exit(1);
  }
  fprintf(stderr, "Loading %s: ", argv[1]);
  o = obj_read(argv[1]);
  fprintf(stderr, "%d vertices, %d faces\n", o->vs->count, o->fs->count);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(800, 600);
  glutCreateWindow(argv[1]);
  for (mi = 0, m = o->mtls->items; mi < o->mtls->count; mi++, m++) {
    if (m->map_kd) {
      snprintf(path, 1000, "%s/%s", dirname(argv[1]), m->map_kd);
      load_texture(mi, path);
    }
  }
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
