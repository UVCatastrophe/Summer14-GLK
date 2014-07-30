TEEM = /home/roman/teem-install
GLM = /home/roman/Summer14-GLK
ATB = ../AntTweakBar

CC = g++ -std=c++11 
FLAGS = -lglfw3 -lGLU -lGL -lX11 -lXxf86vm -lXrandr -lpthread -lXi -Wl,-rpath,$(TEEM)/lib -lteem -lpng -lAntTweakBar -fpermissive
INCLUDE = -I$(TEEM)/include -I$(GLM) -I$(ATB)/include/
LIB = -L$(TEEM)/lib -L$(GLM) -L$(ATB)/lib/
SRC = polydemo.cpp shader.cpp

All:
	$(CC) $(SRC) -o main $(FLAGS) $(LIB) $(INCLUDE)
