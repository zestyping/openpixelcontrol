platform=$(shell uname)

ifeq ($(platform),Darwin)
  GL_OPTS=-framework OpenGL -framework GLUT
  SOIL_OPTS=-framework Carbon -I SOIL
else ifeq ($(platform),Linux)
  GL_OPTS=-lGL -lglut -lGLU
  SOIL_OPTS=-I SOIL
endif

all: objview objtest

objview: objview.c objfile.c
	gcc $(GL_OPTS) $(SOIL_OPTS) -o $@ $^ soil/*.c

objtest: objtest.c objfile.c
	gcc -g -o $@ $^

clean:
	rm -f objview objtest
