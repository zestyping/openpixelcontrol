platform=$(shell uname)

ifeq ($(platform),Darwin)
  ALL=bin/dummy_client bin/dummy_server bin/gl_server
  GL_OPTS=-framework OpenGL -framework GLUT
else ifeq ($(platform),Linux)
  ALL=bin/dummy_client bin/dummy_server bin/tcl_server bin/gl_server
  GL_OPTS=-lGL -lglut -lGLU
endif

all: $(ALL)

clean:
	rm -rf bin/*

bin/dummy_client: dummy_client.c opc_client.c
	mkdir -p bin
	gcc -o $@ $^

bin/dummy_server: dummy_server.c opc_server.c
	mkdir -p bin
	gcc -o $@ $^

bin/tcl_server: tcl_server.c opc_server.c src/spi.c
	mkdir -p bin
	gcc -o $@ $^

bin/gl_server: gl_server.c opc_server.c src/cJSON.c
	mkdir -p bin
	gcc $(GL_OPTS) -o $@ $^
