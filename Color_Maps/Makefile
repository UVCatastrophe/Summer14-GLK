TEEM = /home/glk/roman/teem-install
ATB = /home/glk/roman/AntTweakBar

CC = g++ -std=c++11 
FLAGS = -lglfw3 -lGLU -lGL -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lAntTweakBar -Wl,-rpath,$(TEEM)/lib -lteem -lpng -Wl,-rpath,$(ATB)/lib
INCLUDE = -I$(ATB)/include/ -I$(TEEM)/include
LIB = -L$(ATB)/lib/ -L$(TEEM)/lib
SRC = color_maps.cpp shader.cpp shader.h

All:
	$(CC) $(SRC) -o main $(FLAGS) $(LIB) $(INCLUDE)
