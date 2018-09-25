EXE = raytracer

# Windows
ifeq ($(OS),Windows_NT)
	EXE = raytracer.exe
        LDFLAGS = -lopengl32 -lglu32 -lglut32
   
else
    UNAME_S := $(shell uname -s)
#Linux
    ifeq ($(UNAME_S),Linux)
        LDFLAGS = -lGL -lGLU -lglut
    endif
# OSX
    ifeq ($(UNAME_S),Darwin)
    	LDFLAGS = -framework Carbon -framework OpenGL -framework GLUT
    endif
endif

$(EXE) : raytracer.c
	gcc -o $@ $< $(CFLAGS) $(LDFLAGS)
