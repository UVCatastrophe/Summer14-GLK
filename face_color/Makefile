CC = g++ -std=c++11 
FLAGS = -lglfw3 -lGLU -lGL -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lAntTweakBar -lSOIL
INCLUDE = -I../AntTweakBar/include/
LIB = -L../AntTweakBar/lib/ -L/SOIL/lib/
SRC = face.cpp shader.cpp shader.h

All:
	$(CC) $(SRC) -o main $(FLAGS) $(LIB) $(INCLUDE)
