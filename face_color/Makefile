RT = /Users/ramici
TEEM = $(RT)/teem-install
ATB = $(RT)/AntTweakBar

CC = g++ -std=c++11 

OS := $(shell uname)
ifeq ($(OS), Darwin)
OS_FLAGS = -lglfw -framework CoreVideo  -framework Cocoa -framework OpenGL -framework IOKit 
else
OS_FLAGS = -lglfw3 -lGLU -lGL -lX11 -lXxf86vm -lXrandr -lpthread -lXi
endif

FLAGS = -Wl,-rpath,$(TEEM)/lib -Wl,-rpath,$(ATB)/lib -lteem  -lAntTweakBar -fpermissive

INCLUDE = -I$(ATB)/include/ -I$(TEEM)/include
LIB = -L$(ATB)/lib/ -L$(TEEM)/lib
SRC = face.cpp shader.cpp

OUT = face_color

All:
	$(CC) $(SRC) -o $(OUT) $(OS_FLAGS) $(FLAGS) $(LIB) $(INCLUDE)
