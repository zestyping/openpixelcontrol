/* Copyright 2013 Ka-Ping Yee

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
#include <unistd.h>
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
int verbose = 0;

// Camera parameters
#define FOV_DEGREES 20
int orbiting = 0, dollying = 0;
double start_angle, start_elevation, start_distance;
int start_x, start_y;
double orbit_angle = 192.0;  // camera orbit angle, degrees
double camera_elevation = -15;  // camera elevation angle, degrees
double camera_distance = 16.0;  // distance from origin, metres
double camera_aspect = 1.0;  // will be updated to match window aspect ratio

// Shape parameters
#define SHAPE_THICKNESS 0.06  // thickness of points and lines, metres

#define MAX_CHANNELS 10
int channel_offsets[MAX_CHANNELS];
int num_channels= 0;

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
#define set_rgb(r, g, b) (glColor3d(r, g, b))
colour xfer[256];

// Vector arithmetic
typedef struct {
  double x, y, z;
} vector;

vector tmp_vector;
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

// Shapes
typedef struct shape {
  void (*draw)(struct shape* this, GLUquadric* quad);
  int index;
  union {
    vector point;
    struct { vector start, end; } line;
  } g;
} shape;

#define MAX_SHAPES 30000
int num_shapes = 0;
shape shapes[MAX_SHAPES];

void draw_point(shape* this, GLUquadric* quad) {
  pixel p = pixels[this->index];
  glColor3d(xfer[p.r].r, xfer[p.g].g, xfer[p.b].b);
  glPushMatrix();
  glTranslatef(this->g.point.x, this->g.point.y, this->g.point.z);
  gluSphere(quad, SHAPE_THICKNESS/2, 6, 3);
  glPopMatrix();
}

void draw_line(shape* this, GLUquadric* quad) {
  pixel p = pixels[this->index];
  vector start = this->g.line.start;
  vector delta = subtract(this->g.line.end, this->g.line.start);
  vector z = {0, 0, 1};
  vector hinge = cross(z, delta);
  double len = length(delta);
  double angle = 180./M_PI * acos(dot(z, delta) / len);
  glColor3d(xfer[p.r].r, xfer[p.g].g, xfer[p.b].b);
  glPushMatrix();
  glTranslated(start.x, start.y, start.z);
  glRotated(angle, hinge.x, hinge.y, hinge.z);
  gluSphere(quad, SHAPE_THICKNESS/2, 6, 3);
  gluCylinder(quad, SHAPE_THICKNESS/2, SHAPE_THICKNESS/2, len, 6, 1);
  glTranslated(0, 0, len);
  gluSphere(quad, SHAPE_THICKNESS/2, 6, 3);
  glPopMatrix();
}

void draw_axes() {
  vector o = {0, 0, 0};
  vector x = {1, 0, 0};
  vector y = {0, 1, 0};
  vector z = {0, 0, 1};
  vector xx = {10, 0, 0};
  vector yy = {0, 10, 0};
  vector zz = {0, 0, 10};
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

void display() {
  int i;
  shape* sh;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  draw_axes();
  GLUquadric* quad = gluNewQuadric();
  for (i = 0, sh = shapes; i < num_shapes; i++, sh++) {
    sh->draw(sh, quad);
  }
  gluDeleteQuadric(quad);
  glutSwapBuffers();
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
  int i = 0, j = 0;

  if (verbose) {
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
  }

  if (channel > num_channels) {
    return;
  }
  for (i = 0; i < count; i++) {
    if (channel == 0) {
      // Channel 0 is broadcast
      for (j = 0; j < num_channels; j++) {
        pixels[i + channel_offsets[j]] = p[i];
      }
    } else {
      pixels[i + channel_offsets[channel-1]] = p[i];
    }
  }
}

void idle() {
  /*
   * Receive all pending frames. We'll often draw slower than an OPC source
   * is producing pixels; to avoid runaway lag due to data buffered in the socket,
   * we want to skip frames.
   *
   * A short timeout (20 ms) on the first receive keeps us responsive to mouse events.
   * A zero timeout on subsequent receives lets us drain any queued frames without
   * waiting for them.
   */

  if (opc_receive(source, handler, 20) > 0) {

    // Drain queue
    while (opc_receive(source, handler, 0) > 0);

    // Show the last received frame
    display();
  }
}

char* read_file(char* filename) {
  FILE* fp;
  struct stat st;
  char* buffer;

  if (stat(filename, &st) != 0) {
	  return NULL;
  }
  buffer = malloc(st.st_size + 1);
  fp = fopen(filename, "r");
  fread(buffer, st.st_size, 1, fp);
  fclose(fp);
  buffer[st.st_size] = 0;
  return buffer;
}

void load_layout(char* filename, int channel) {
  char* buffer;
  cJSON* json;
  cJSON* item;
  cJSON* index;
  cJSON* point;
  cJSON* x;
  cJSON* line;
  cJSON* start;
  cJSON* x2;
  int i = 0;
  
  buffer = read_file(filename);
  if (buffer == NULL) {
	  fprintf(stderr, "Unable to open '%s'\n", filename);
	  exit(1);
  }
  json = cJSON_Parse(buffer);
  if (json == NULL) {
	  fprintf(stderr, "Unable to parse '%s'\n", filename);
	  exit(1);
  }
  free(buffer);
  channel_offsets[channel] = num_pixels;
  if (verbose) {
    printf("Channel %d offset is %d\n", channel, channel_offsets[channel]);
  }
  fprintf(stderr, "Loaded \"%s\" as channel %d\n", filename, channel + 1);

  int shape_count = 0;
  for (item = json->child, i = 0; item; item = item->next, i++) {
    index = cJSON_GetObjectItem(item, "index");
    if (index) {
      i = index->valueint;
    }
    point = cJSON_GetObjectItem(item, "point");
    x = point ? point->child : NULL;
    if (x && x->next && x->next->next) {
      shapes[num_shapes].draw = draw_point;
      shapes[num_shapes].index = channel_offsets[channel] + i;
      shapes[num_shapes].g.point.x = x->valuedouble;
      shapes[num_shapes].g.point.y = x->next->valuedouble;
      shapes[num_shapes].g.point.z = x->next->next->valuedouble;
      num_shapes++;
      shape_count++;
    }
    line = cJSON_GetObjectItem(item, "line");
    start = line ? line->child : NULL;
    x = start ? start->child : NULL;
    x2 = start && start->next ? start->next->child : NULL;
    if (x && x->next && x->next->next && x2 && x2->next && x2->next->next) {
      shapes[num_shapes].draw = draw_line;
      shapes[num_shapes].index = channel_offsets[channel] + i;
      shapes[num_shapes].g.line.start.x = x->valuedouble;
      shapes[num_shapes].g.line.start.y = x->next->valuedouble;
      shapes[num_shapes].g.line.start.z = x->next->next->valuedouble;
      shapes[num_shapes].g.line.end.x = x2->valuedouble;
      shapes[num_shapes].g.line.end.y = x2->next->valuedouble;
      shapes[num_shapes].g.line.end.z = x2->next->next->valuedouble;
      num_shapes++;
      shape_count++;
    }
  }
  num_pixels += shape_count;
  for (i = channel_offsets[channel]; i < shape_count; i++) {
    pixels[i].r = pixels[i].g = pixels[i].b = 1;
  }
}

void init(char** filenames, int total_channels) {
  int channel = 0, i = 0;
  for (channel=0; channel < total_channels; channel++) {
    load_layout(filenames[channel], channel);
  }
  for (i = 0; i < 256; i++) {
    xfer[i].r = xfer[i].g = xfer[i].b = 0.1 + i*0.9/256;
  }
}

void usage(char* prog_name) {
  fprintf(stderr, "Usage: %s <options> -l <filename.json> [<port>]\n", prog_name);
  exit(1);
}

int main(int argc, char** argv) {
  u16 port;

  glutInit(&argc, argv);

  int iflag = 0;
  u8 channel = 1;
  enum { WORD_MODE, LINE_MODE } op_mode = WORD_MODE;  // Default set
  int opt;
  char* layouts[MAX_CHANNELS];

  while ((opt = getopt(argc, argv, ":l:p:")) != -1)
  {
      switch (opt)
      {
      case 'l':
          num_channels += 1;
          if (num_channels > MAX_CHANNELS) {
              fprintf(stderr, "Can only simulate up to %d channels", MAX_CHANNELS);
              exit(1);
          }
          layouts[num_channels - 1] = optarg;
          break;
      case 'p':
          port = strtol(optarg, NULL, 10);
          break;
      default:
          usage(argv[0]);
      }
  }
  if (num_channels == 0) {
      usage(argv[0]);
  }
  init(layouts, num_channels);
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
