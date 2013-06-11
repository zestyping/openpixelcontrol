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

#include "cJSON.h"
#include "opc.h"

opc_source source = -1;

// Camera parameters
#define FOV_DEGREES 20
int orbiting = 0, dollying = 0;
double start_angle, start_elevation, start_distance;
int start_x, start_y;
double orbit_angle = 192.0;  // camera orbit angle, degrees
double camera_elevation = -15;  // camera elevation angle, degrees
double camera_distance = 16.0;  // metres
double camera_aspect = 1.0;

// LED colours
#define MAX_PIXELS 30000
int num_pixels = 0;
pixel pixels[MAX_PIXELS];

// Floating-point colours
typedef struct {
  double r, g, b;
} colour;

colour tmp_colour;
#define set_colour(c) ((tmp_colour = c), glColor3dv(&(tmp_colour.r)))
colour xfer[256];

// Vector arithmetic
typedef struct {
  double x, y, z;
} vector;

vector tmp_vector;
vector vectors[MAX_PIXELS];
#define put_vertex(v) ((tmp_vector = v), glVertex3dv(&(tmp_vector.x)))

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

void draw_axes() {
  vector o = {0, 0, 0};
  vector x = {1, 0, 0};
  vector y = {0, 1, 0};
  vector z = {0, 0, 1};
  colour r = {0.3, 0, 0};
  colour g = {0, 0.3, 0};
  colour b = {0, 0, 0.3};
  colour w = {0.3, 0.3, 0.3};
  glLineWidth(2);
  glBegin(GL_LINES);
  set_colour(w);
  put_vertex(o);
  put_vertex(x);
  put_vertex(o);
  put_vertex(y);
  put_vertex(o);
  put_vertex(z);
  set_colour(r);
  put_vertex(x);
  put_vertex(multiply(10, x));
  set_colour(g);
  put_vertex(y);
  put_vertex(multiply(10, y));
  set_colour(b);
  put_vertex(z);
  put_vertex(multiply(10, z));
  glEnd();
}

void display() {
  int i;
  GLUquadric* quad = gluNewQuadric();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  draw_axes();
  for (i = 0; i < num_pixels; i++) {
    glColor3d(xfer[pixels[i].r].r, xfer[pixels[i].g].g, xfer[pixels[i].b].b);
    glPushMatrix();
    glTranslatef(vectors[i].x, vectors[i].y, vectors[i].z);
    gluSphere(quad, 0.03, 6, 3);
    glPopMatrix();
  }
  glutSwapBuffers();

  gluDeleteQuadric(quad);
}

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

void handler(u8 channel, u16 count, pixel* p) {
  int i = 0;
  char* sep = " =";
  printf("-> channel %d: %d pixel%s", channel, count, count == 1 ? "" : "s");
  for (i = 0; i < count; i++) {
    if (i >= 4) {
      printf(", ...");
      break;
    }
    printf("%s %02x %02x %02x", sep, p[i].r, p[i].g, p[i].b);
    sep = ",";
  }
  printf("\n");
  for (i = 0; i < count; i++) {
    pixels[i] = p[i];
  }
  display();
}

void idle() {
  /* A short timeout (20 ms) keeps us responsive to mouse events. */
  opc_receive(source, handler, 20);
}

char* read_file(char* filename) {
  FILE* fp;
  struct stat st;
  char* buffer;

  if (stat(filename, &st) != 0) {
    return strdup("");
  }
  buffer = malloc(st.st_size + 1);
  fp = fopen(filename, "r");
  fread(buffer, st.st_size, 1, fp);
  fclose(fp);
  buffer[st.st_size] = 0;
  return buffer;
}

void init(char* filename) {
  char* buffer;
  cJSON* json;
  cJSON* item;
  cJSON* index;
  cJSON* point;
  cJSON* x;
  int i = 0;
  
  buffer = read_file(filename);
  json = cJSON_Parse(buffer);
  free(buffer);

  for (item = json->child, i = 0; item; item = item->next, i++) {
    index = cJSON_GetObjectItem(item, "index");
    if (index) {
      i = index->valueint;
    }
    point = cJSON_GetObjectItem(item, "point");
    x = point ? point->child : NULL;
    if (x && x->next && x->next->next) {
      vectors[i].x = x->valuedouble;
      vectors[i].y = x->next->valuedouble;
      vectors[i].z = x->next->next->valuedouble;
    }
  }
  num_pixels = i;
  for (i = 0; i < num_pixels; i++) {
    pixels[i].r = pixels[i].g = pixels[i].b = 1;
  }
  for (i = 0; i < 256; i++) {
    xfer[i].r = xfer[i].g = xfer[i].b = 0.1 + i*0.9/256;
  }
}

int main(int argc, char** argv) {
  u16 port;

  glutInit(&argc, argv);
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <options> <filename.json> [<port>]\n", argv[0]);
    exit(1);
  }
  init(argv[1]);
  port = argc > 2 ? strtol(argv[2], NULL, 10) : 0;
  port = port ? port : OPC_DEFAULT_PORT;
  source = opc_new_source(port);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("OPC");
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutIgnoreKeyRepeat(1);
  glutKeyboardFunc(keyboard);
  glutIdleFunc(idle);

  glEnable(GL_DEPTH_TEST);
#ifdef __APPLE__
  /* Make glutSwapBuffers wait for vertical refresh to avoid frame tearing. */
  int swap_interval = 1;
  CGLContextObj context = CGLGetCurrentContext();
  CGLSetParameter(context, kCGLCPSwapInterval, &swap_interval);
#endif

  glutMainLoop();
  return 0;
}
