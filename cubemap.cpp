//cs371 Fall 2013
//program: cubemap.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: framework for ray tracing
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "log.h"
#include "ppm.h"
#include "extern.h"

void buildCubemapTexture(char *filename)
{
	Log("void build_cubemap_texture(%s)...\n",filename);
	//
	Ppmimage *ppmimage = ppm6GetImage(filename);
	g.cubemap.data = ppmimage->data;
	g.cubemap.sideres = ppmimage->width/4;
	//
	//int i,j;
	//unsigned char *p = g.cubemap.ppmimage->data;
	//for (i=0; i<g.cubemap.sideres; i++) {
	//	for (j=0; j<g.cubemap.sideres; j++) {
	//		Log("%i %i %i\n",*(p+0),*(p+1),*(p+2));
	//		p+=3;
	//	}
	//}
	//Log("g.cubemap.sideres: %i\n",g.cubemap.sideres);
}

void adjustCubeAngle(void)
{
	Log("adjust_cube_angle()...\n");
	g.studio.angle=90.0;
	MakeVector(0,1,0,g.studio.up);
	if (!g.cubemap.cnt) {
		VecCopy(g.studio.from, g.cubemap.from);
	}
	VecCopy(g.cubemap.from, g.studio.from);
	VecCopy(g.cubemap.from, g.studio.at);
	switch(g.cubemap.cnt) {
		case 0:
			g.studio.at[0] -= 1.0;
			break;
		case 1:
			g.studio.at[2] -= 1.0;
			break;
		case 2:
			g.studio.at[0] += 1.0;
			break;
		case 3:
			g.studio.at[2] += 1.0;
			break;
		case 4:
			//note: never look exactly straight down or up
			g.studio.at[1] += 1.0;
			g.studio.at[2] -= 0.000001;
			break;
		case 5:
			g.studio.at[1] -= 1.0;
			g.studio.at[2] -= 0.000001;
			break;
	}
}

//1=long
//2=cross
#define CUBEMAP_TYPE 2

void buildCube(void)
{
	printf("build_cube()...\n");
	static float *cubeBits=NULL;
	if (g.cubemap.cnt >= 6) return;
	if (!cubeBits) {
		int bitsize = g.screen.yres * g.screen.xres3 * sizeof(float) * 6;
		if (CUBEMAP_TYPE == 2)
			bitsize = g.screen.yres * g.screen.xres3 * sizeof(float) * 4 * 3;
		cubeBits = (float *)malloc(bitsize);
		//Log("cubebits was allocated!\n");
	}
	//This is for a long bitmap image
	float *p = cubeBits + (g.screen.xres3 * g.cubemap.cnt);
	//
	if (CUBEMAP_TYPE == 2) {
		//cross type bitmap image
		if (g.cubemap.cnt <= 3) {
			//sides
			p = cubeBits + (g.screen.yres * g.screen.xres3 * 4 +
			    g.screen.xres3 * g.cubemap.cnt);
		}
		if (g.cubemap.cnt == 4) {
			//look up
			p = cubeBits + (g.screen.yres * g.screen.xres3 * 8 +
			    g.screen.xres3);
		}
		if (g.cubemap.cnt == 5) {
			//look down
			p = cubeBits + g.screen.xres3;
		}
	}
	float *s = g.screendata;
	int i,j;
	for (i=0; i<g.screen.yres; i++) {
		memcpy(p, s, g.screen.xres3 * sizeof(float));
		j=6;
		if (CUBEMAP_TYPE == 2)
			j=4;
		p += (g.screen.xres3*j);
		s += (g.screen.xres3);
	}
	if (g.cubemap.cnt==5) {
		char cc[]="convert scenes/mycube.ppm -quality 40 scenes/mycube.jpg";
		//save the image
		Ppmimage *image;
		if (CUBEMAP_TYPE == 1) {
			//long 6x1 cubemap image
			image = ppm6CreateImage(g.screen.xres*6, g.screen.yres);
			floatdataToChar(cubeBits, (unsigned char *)image->data,
			                  g.screen.xres*6, g.screen.yres);
			ppm6SaveImage(image, (char *)"scenes/mycube.ppm");
			ppm6CleanupImage(image);
			//convert to png. call convert command.
			if (system(cc)) {
				printf("image error!\n");
			}
			remove("scenes/mycube.ppm");
		}
		if (CUBEMAP_TYPE == 2) {
			//cross-type 4x3 cubemap image
			image = ppm6CreateImage(g.screen.xres*4, g.screen.yres*3);
			floatdataToChar(cubeBits, (unsigned char *)image->data,
			                  g.screen.xres*4, g.screen.yres*3);
			ppm6SaveImage(image, (char *)"scenes/mycube.ppm");
			ppm6CleanupImage(image);
			//convert to png. call convert command.
			if (system(cc)) {
				printf("image error!\n");
			}
			remove("scenes/mycube.ppm");
		}
		free(cubeBits);
		cubeBits=NULL;
	}
	g.cubemap.cnt++;
}



