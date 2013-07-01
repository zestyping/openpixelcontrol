platform=$(shell uname)

ifeq ($(platform),Darwin)
  GL_OPTS=-framework OpenGL -framework GLUT
else ifeq ($(platform),Linux)
  GL_OPTS=-lGL -lglut -lGLU
endif

all: objview objtest

objview: objview.c objfile.c
	gcc $(GL_OPTS) -o $@ $^

objtest: objtest.c objfile.c
	gcc -g -o $@ $^

clean:
	rm -f objview objtest
