TEEM = /home/roman/teem-install
GLM = /home/roman/Summer14-GLK

CC = g++ -std=c++11 
FLAGS = -lglfw3 -lGLU -lGL -lX11 -lXxf86vm -lXrandr -lpthread -lXi -Wl,-rpath,$(TEEM)/lib -lteem -lpng 
INCLUDE = -I$(TEEM)/include -I$(GLM)
LIB = -L$(TEEM)/lib -L$(GLM)
SRC = polydemo.cpp shader.cpp shader.h

All:
	$(CC) $(SRC) -o main $(FLAGS) $(LIB) $(INCLUDE)
