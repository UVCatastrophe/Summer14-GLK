TEEM = /home/roman/teem-install

CC = g++ -std=c++11 
FLAGS = -lglfw3 -lGLU -lGL -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lAntTweakBar -Wl,-rpath,$(TEEM)/lib -lteem -lpng
INCLUDE = -I../AntTweakBar/include/ -I$(TEEM)/include
LIB = -L../AntTweakBar/lib/ -L$(TEEM)/lib
SRC = face.cpp shader.cpp shader.h

All:
	$(CC) $(SRC) -o main $(FLAGS) $(LIB) $(INCLUDE)
