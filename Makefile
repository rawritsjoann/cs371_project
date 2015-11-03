## If you receive XRR errors, include -lXrandr option on LFLAGS line
all: atrace
##LIB    = ./libggfonts.so
LIB    = ./libggfonts.a
LFLAGS = -lX11 -lGLU -lGL -lm #-lXrandr
CFLAGS = -O2 -Wall -Wextra
SOURCE = atrace.cpp init.cpp scene.cpp render.cpp sphere.cpp ppm.cpp log.cpp \
			vector.cpp input.cpp files.cpp perlin.cpp texture.cpp divide.cpp \
			cubemap.cpp matrix.cpp fresnel.cpp haze.cpp animate.cpp \
			photons.cpp subsurf.cpp

#all: atrace atrace.o init.o scene.o render.o sphere.o ppm.o log.o \
#	vector.o input.o files.o perlin.o texture.o divide.o \
#	cubemap.o matrix.o fresnel.o haze.o animate.o
#
#atrace: $(SOURCE) fonts.h defs.h ppm.h extern.h bvh.h animate.h $(LIB)
#	g++ $(CFLAGS) $(SOURCE) $(LFLAGS) $(LIB) -o atrace

atrace: atrace.o init.o scene.o render.o sphere.o ppm.o log.o \
vector.o input.o files.o perlin.o texture.o divide.o \
cubemap.o matrix.o fresnel.o haze.o animate.o photons.o subsurf.o \
defs.h ppm.h extern.h bvh.h animate.h
	g++ atrace.o init.o scene.o render.o sphere.o ppm.o log.o vector.o \
input.o files.o perlin.o texture.o divide.o cubemap.o matrix.o fresnel.o \
haze.o animate.o photons.o subsurf.o $(LIB) -oatrace $(LFLAGS)


atrace.o: atrace.cpp defs.h ppm.h extern.h bvh.h animate.h
	g++ atrace.cpp -c $(CFLAGS) $(LFLAGS)
init.o: init.cpp defs.h
	g++ init.cpp -c $(CFLAGS) $(LFLAGS)
scene.o: scene.cpp defs.h
	g++ scene.cpp -c $(CFLAGS) $(LFLAGS)
render.o: render.cpp defs.h
	g++ render.cpp -c $(CFLAGS) $(LFLAGS)
sphere.o: sphere.cpp defs.h
	g++ sphere.cpp -c $(CFLAGS) $(LFLAGS)
ppm.o: ppm.cpp defs.h ppm.h
	g++ ppm.cpp -c $(CFLAGS) $(LFLAGS)
log.o: log.cpp defs.h
	g++ log.cpp -c $(CFLAGS) $(LFLAGS)
vector.o: vector.cpp defs.h
	g++ vector.cpp -c $(CFLAGS) $(LFLAGS)
input.o: input.cpp defs.h
	g++ input.cpp -c $(CFLAGS) $(LFLAGS)
files.o: files.cpp defs.h files.h
	g++ files.cpp -c $(CFLAGS) $(LFLAGS)
perlin.o: perlin.cpp defs.h
	g++ perlin.cpp -c $(CFLAGS) $(LFLAGS)
texture.o: texture.cpp defs.h
	g++ texture.cpp -c $(CFLAGS) $(LFLAGS)
divide.o: divide.cpp defs.h bvh.h
	g++ divide.cpp -c $(CFLAGS) $(LFLAGS)
cubemap.o: cubemap.cpp defs.h
	g++ cubemap.cpp -c $(CFLAGS) $(LFLAGS)
matrix.o: matrix.cpp defs.h
	g++ matrix.cpp -c $(CFLAGS) $(LFLAGS)
fresnel.o: fresnel.cpp defs.h
	g++ fresnel.cpp -c $(CFLAGS) $(LFLAGS)
haze.o: haze.cpp defs.h
	g++ haze.cpp -c $(CFLAGS) $(LFLAGS)
animate.o: animate.cpp defs.h animate.h ppm.h
	g++ animate.cpp -c $(CFLAGS) $(LFLAGS)
photons.o: photons.cpp defs.h
	g++ photons.cpp -c $(CFLAGS) $(LFLAGS)
subsurf.o: subsurf.cpp defs.h
	g++ subsurf.cpp -c $(CFLAGS) $(LFLAGS)

clean:
	rm -f atrace
	rm -f *.o

