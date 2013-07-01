#include "objfile.h"

int main(int argc, char* argv[]) {
  FILE* fp = fopen(argv[1], "rt");
  obj* o = obj_read(fp);
  fclose(fp);

  fp = fopen(argv[2], "wt");
  obj_write(o, fp);
  fclose(fp);

  obj_free(o);
  return 0;
}
