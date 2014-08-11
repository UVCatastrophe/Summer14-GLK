TEEM = /home/roman/teem-install
GLM = /home/roman/Summer14-GLK
ATB = ../AntTweakBar


CC = g++ -std=c++11 
FLAGS = -lglfw3 -lGLU -lGL -lX11 -lXxf86vm -lXrandr -lpthread -lXi -Wl,-rpath,$(TEEM)/lib -lteem -lpng -lAntTweakBar -fpermissive
INCLUDE = -I$(TEEM)/include -I$(GLM) -I$(ATB)/include/
LIB = -L$(TEEM)/lib -L$(GLM) -L$(ATB)/lib/

SRC_POLYDEMO = polydemo.cpp shader.cpp
OUT_POLYDEMO = -o polydemo

SRC_ISORENDER = isorender.cpp shader.cpp
OUT_ISORENDER = -o isorender

All:
	$(CC) $(SRC_POLYDEMO) -o main $(FLAGS) $(LIB) $(INCLUDE)
poly:
	$(CC) $(SRC_POLYDEMO) $(OUT_POLYDEMO) $(FLAGS) $(LIB) $(INCLUDE)
iso:
	$(CC) $(SRC_ISORENDER) $(OUT_ISORENDER) $(FLAGS) $(LIB) $(INCLUDE)
