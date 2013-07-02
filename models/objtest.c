#include "objfile.h"

int main(int argc, char* argv[]) {
  obj* o = obj_read(argv[1]);
  obj_write(o, argv[2]);
  obj_free(o);
  return 0;
}
