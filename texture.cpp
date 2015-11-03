//cs371 Introduction To Computer Graphics
//program: texture.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: a framework for ray tracing
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "defs.h"
#include "log.h"
#include "vector.h"
#include "extern.h"

#define XHUGE 9e9
#define MINZ(a,b) ((a)<(b))?(a):(b)

void checkerPattern(Vec P, Surface *sf, Vec rgb) {
	Cpattern *cpat = &g.cpattern[sf->checker_idx];
	int i, p[3]={0,0,0};
	Flt *fptr = cpat->color0;
	Flt pt, blur = XHUGE, tex_blur = cpat->groutWidth;
	//Flt blur = XHUGE, tex_blur = 0.1;
	Vec P1;
	transVector(sf->invmat, P, P1);
	for (i=0; i<3; i++) {
		//point[i] = 0.0;
		if (cpat->scale[i] > 0.0) {
			pt = P1[i] / cpat->scale[i];
			p[i] = (int)floor(pt);
			if (cpat->grout) {
				pt -= p[i];
				if (pt > 0.5)
					pt = 1.0 - pt;
				blur = MINZ(pt, blur);
			}
		}
	}
	if ((p[0]+p[1]+p[2]) & 0x01)
		fptr = cpat->color1;
	if (cpat->grout) {
		blur *= 2.0;
		if (blur < tex_blur) {
			blur = (1.0 - blur/tex_blur) * 0.5;
			fptr = cpat->color2;
			//tint the grout
			//rgb[0] *= .5;
			//rgb[1] *= .5;
			//rgb[2] *= .5;
		}
	}
	VecCopy(fptr, rgb);
}

float turbulence(float vec[3], int power) {
	//Add turbulence to Perlin noise
	float tv[3];
	float value=0.0f;
	float f;
	float fp = pow(2.0, power);
	for (f=1.0f; f<=fp; f*=2.0f) {
		tv[0] = vec[0] * f;
		tv[1] = vec[1] * f;
		tv[2] = vec[2] * f;
		value += noise3(tv) / f;
	}
	return value;
}

void backgroundHit(Ray *ray, Vec rgb)
{
	//Ray did not hit an object
	//ray_d = direction of ray
	//rgb   = color to be returned
	Vec ray_d = {ray->d[0],ray->d[1],ray->d[2]};

	switch(g.studio.sky.type) {
		case SKY_TYPE_BLUE:
			//Color gets more blue as vector points upward.
			rgb[0] = 0.3;
			rgb[1] = 0.4;
			rgb[2] = ((ray_d[1] + 1.0) / 1.0);
			break;
		case SKY_TYPE_PERLIN: {
			float f1;
			Vec p;
			//The ray direction determines the perlin value
			p[0] = ray_d[0] * g.studio.sky.scale[0];
			p[1] = ray_d[1] * g.studio.sky.scale[1];
			p[2] = ray_d[2] * g.studio.sky.scale[2];
			float vf[3] = { (float)p[0],
							(float)p[1],
							(float)p[2] };
			float fval = noise3(vf);
			fval *= 1.4;
			if (ray_d[1] <= g.studio.sky.horizon) {
				//fade clouds out near horizon
				float a = ray_d[1] * 1.0 / g.studio.sky.horizon;
				a = a < 0.0 ? 0.0 : a;
				fval *= a;
			}
			f1 = 1.0f - fval;
			rgb[0] = g.studio.sky.color[0][0] *
			         fval + g.studio.sky.color[1][0] * f1;
			rgb[1] = g.studio.sky.color[0][1] *
			         fval + g.studio.sky.color[1][1] * f1;
			rgb[2] = g.studio.sky.color[0][2] *
			         fval + g.studio.sky.color[1][2] * f1;
			break;
		}
		case SKY_TYPE_ENVIRONMENT: {
			//Get the color from the current cubemap
			unsigned char *p;
			int res = g.cubemap.sideres;
			int res3 = res*3;
			rgb[0] = 1.0f;
			rgb[1] = 0.0f;
			rgb[2] = 0.0f;
			VecNormalize(ray_d);
			//return;
			//
			//http://www.codermind.com/articles/Raytracer-in-C++-Part-III-Textures.html
			//
			Flt d0 = fabs(ray_d[0]);
			Flt d1 = fabs(ray_d[1]);
			Flt d2 = fabs(ray_d[2]);
			if (d0 > d1 && d0 > d2) {
				//left, right
				if (ray_d[0] > 0.0) {
					//right
					//corner of side
					Flt zf = ( ray_d[2] / ray_d[0] + 1.0) * 0.5;
					Flt yf = (-ray_d[1] / ray_d[0] + 1.0) * 0.5;
					int yi = (int)(yf * (Flt)res);
					int zi = (int)(zf * (Flt)res);
					p = (unsigned char *)g.cubemap.data + res*res3*4 +
										(yi * res3*4 + zi * 3);
					rgb[0] = (float)*p / 255.0; p++;
					rgb[1] = (float)*p / 255.0; p++;
					rgb[2] = (float)*p / 255.0;
				} else {
					//left
					//corner of side
					Flt zf = (ray_d[2] / ray_d[0] + 1.0) * 0.5;
					Flt yf = (ray_d[1] / ray_d[0] + 1.0) * 0.5;
					int yi = (int)(yf * (Flt)res);
					int zi = (int)(zf * (Flt)res);
					p = (unsigned char *)g.cubemap.data + res*res3*4+res3*2 +
										(yi * res3*4 + zi * 3);
					rgb[0] = (float)*p / 255.0; p++;
					rgb[1] = (float)*p / 255.0; p++;
					rgb[2] = (float)*p / 255.0;
				}
			}
			if (d1 > d0 && d1 > d2) {
				//top, bottom
				if (ray_d[1] > 0.0) {
					//top
					unsigned char *p;
					//corner of side
					Flt xf = (-ray_d[0] / ray_d[1] + 1.0) * 0.5;
					Flt zf = (ray_d[2] / ray_d[1] + 1.0) * 0.5;
					int xi = (int)(xf * (Flt)res);
					int zi = (int)(zf * (Flt)res);
					p = (unsigned char *)g.cubemap.data + res3 +
										(zi * res3*4 + xi * 3);
					rgb[0] = (float)*p / 255.0; p++;
					rgb[1] = (float)*p / 255.0; p++;
					rgb[2] = (float)*p / 255.0;
				} else {
					//bottom
					unsigned char *p;
					//corner of side
					Flt xf = (ray_d[0] / ray_d[1] + 1.0) * 0.5;
					Flt zf = (ray_d[2] / ray_d[1] + 1.0) * 0.5;
					int xi = (int)(xf * (Flt)res);
					int zi = (int)(zf * (Flt)res);
					p = (unsigned char *)g.cubemap.data + res*res3*8+res3 +
										(zi * res3*4 + xi * 3);
					rgb[0] = (float)*p / 255.0; p++;
					rgb[1] = (float)*p / 255.0; p++;
					rgb[2] = (float)*p / 255.0;
				}

			}
			if (d2 > d0 && d2 > d1) {
				//front, back
				if (ray_d[2] > 0.0) {
					//back
					unsigned char *p;
					//corner of side
					Flt xf = (-ray_d[0] / ray_d[2] + 1.0) * 0.5;
					Flt yf = (-ray_d[1] / ray_d[2] + 1.0) * 0.5;
					int xi = (int)(xf * (Flt)res);
					int yi = (int)(yf * (Flt)res);
					p = (unsigned char *)g.cubemap.data + res*res3*4+res3*1 +
											(yi * res3*4 + xi * 3);
					rgb[0] = (float)*p / 255.0; p++;
					rgb[1] = (float)*p / 255.0; p++;
					rgb[2] = (float)*p / 255.0;
				} else {
					//front
					//corner of side
					Flt xf = (-ray_d[0] / ray_d[2] + 1.0) * 0.5;
					Flt yf = (ray_d[1] / ray_d[2] + 1.0) * 0.5;
					int xi = (int)(xf * (Flt)res);
					int yi = (int)(yf * (Flt)res);
					p = (unsigned char *)g.cubemap.data + res*res3*4+res3*3 +
											(yi * res3*4 + xi * 3);
					rgb[0] = (float)*p / 255.0; p++;
					rgb[1] = (float)*p / 255.0; p++;
					rgb[2] = (float)*p / 255.0;
				}

			}
			break;
		}
		default:
			VecCopy(g.studio.background, rgb);
			break;
	}
	if (g.studio.hazeOn) {
	//	extern void testForHaze(Vec P, const Vec I, Flt t, Vec col,
	//											const int level, int hitYN);
		//extern void add_haze_to_color(Hit *hit, Vec c);
		//add_haze_to_color(&closehit, surfaceColor);
		//P = intersection point, where ray hit an object.
		//I = direction of ray from camera.
		//hitYN == 1 when ray hits anything but the background
		//t = distance to where the ray hit
	//	Vec p = {	ray->o[0] + ray->d[0]*10000.0,
	//				ray->o[1] + ray->d[1]*10000.0,
	//				ray->o[2] + ray->d[2]*10000.0 };
	//	testForHaze(p, ray->d, 10000.0, rgb, 0, 0);

		extern void testForHaze(Ray *ray, Flt t, Vec col);
		testForHaze(ray, 100000.0, rgb);
	}
	//
	//if (g.studio.haze.on) {
	//	//VecCopy(g.studio.haze.color, rgb);
	//	Hit h;
	//	h.t = 9e9;
	//	add_haze_to_color(&h, rgb);
	//}
}

void perlin_texture_2D(Vec p, Surface *sf, Vec rgb)
{
	//perlin noise 2D
	//p   = hit point
	//sf  = surface characteristics
	//rgb = color to be returned
	//
	Vec p1;
	transVector(sf->invmat, p, p1);
	//
	float vf[2] = { (float)p1[0]*(float)sf->perlin_scale[0],
					(float)p1[2]*(float)sf->perlin_scale[2] };
	//Call the Perlin noise function...
	float f1 = noise2(vf);
	//range is around -0.66 to 0.66
	//so expand
	f1 *= 1.4;
	//f2 is second color component
	float f2 = 1.0f - f1;
	rgb[0] = sf->perlin_color[0][0] * f1 + sf->perlin_color[1][0] * f2;
	rgb[1] = sf->perlin_color[0][1] * f1 + sf->perlin_color[1][1] * f2;
	rgb[2] = sf->perlin_color[0][2] * f1 + sf->perlin_color[1][2] * f2;
}

void perlin_texture_3D(Vec p, Surface *sf, Vec rgb)
{
	//perlin noise 3D
	//p   = hit point
	//sf  = surface characteristics
	//rgb = color to be returned
	//
	Vec p1;
	transVector(sf->invmat, p, p1);
	//
	//For marble, try cos(x + noise3(v))
	float vf[3] = { (float)p1[0]*(float)sf->perlin_scale[0],
					(float)p1[1]*(float)sf->perlin_scale[1],
					(float)p1[2]*(float)sf->perlin_scale[2] };
	//float fval = noise3(vf);
	float f1 = turbulence(vf, 3);
	//float fval = cos(vf[0] + turbulence(vf));
	//float fval = cos(vf[0] + noise3(vf));
	//
	//range is around -0.5 to 0.5
	//so expand
	f1 *= 1.5;
	//f2 is second color component
	float f2 = 1.0f - f1;
	rgb[0] = sf->perlin_color[0][0] * f1 + sf->perlin_color[1][0] * f2;
	rgb[1] = sf->perlin_color[0][1] * f1 + sf->perlin_color[1][1] * f2;
	rgb[2] = sf->perlin_color[0][2] * f1 + sf->perlin_color[1][2] * f2;
}


//http://physbam.stanford.edu/cs448x/old/Procedural_Noise%282f%29Perlin_Noise.html
//function x = turbulence(pos, pixel_size)
//  x=0;
//  scale = 1;
//  while(scale>pixel_size)
//      pos = pos/scale;
//      x = x + noise(pos)*scale;
//      scale = scale/2;
//  end;
//
//function rgb = marble(pos)
//  rgb = zeros(1,3);
//  x = sin((pos(2)+3.0*turbulence(pos,0.0125))*pi);
//  x = sqrt(x+1)*.7071;
//  rgb(2)=.3+.8*x;
//  x = sqrt(x);
//  rgb(1)= .3 + .6*x;
//  rgb(3)= .6 + .4*x;


void resolveTexmap(Object *o, Hit *hit)
{
	Tri *t = (Tri *)o->obj;
	Surface *sf = o->surface;
	Flt ttx[2];
	Flt u = t->uvw[0];
	Flt v = t->uvw[1];
	Flt w = t->uvw[2];
	int j;
	static Flt PItimes2 = PI * 2.0;
	static Flt PIdiv2   = PI * 0.5;

	//this can be optimized
	if (sf->spherical) {
		//The color is from a spherical texture map
		//Use the hit-normal to get the tex coordinates.
		int m2;
		Flt oo256 = 1.0 / 255.0;
		Flt tx,ty;

		Vec vn;
		//VecCopy(hit->nhit, vn);
		transNormal(sf->mat, hit->nhit, vn);

		Flt angle1 = PI - (atan2(vn[2], vn[0]));
		int width  = sf->tm->xres;
		int height = sf->tm->yres;
		//Log("resolve_texmap()...\n");
		//Log("angle1: %lf\n",angle1);
		//Log("wh: %i %i\n",width,height);
		tx = ((Flt)width * (angle1 / PItimes2));
		m2 = (Flt)(height/2);
		//
		Flt angle2 = asin(vn[1]);
		if (hit->norm[1] < 0.0) {
			ty = m2 + (m2 * (-angle2 / PIdiv2));
		} else {
			ty = m2 - (m2 * (angle2 / PIdiv2));
		}
		int j = (int)floor(tx + 0.00001);
		int i = (int)floor(ty + 0.00001);
		int offset = i * width * 3 + j * 3;
		unsigned char *ptr = sf->tm->data + offset;
		hit->color[0] = (Flt)(*(ptr+0)) * oo256;
		hit->color[1] = (Flt)(*(ptr+1)) * oo256;
		hit->color[2] = (Flt)(*(ptr+2)) * oo256;
		//Log("col: %lf %lf %lf\n",hit->color[0],hit->color[1],hit->color[2]);
		return;
	}



	//Log("coo: %f %f  %f %f  %f %f\n",
	//					t->texcoord[0][0],
	//					t->texcoord[1][0],
	//					t->texcoord[2][0],
	//					t->texcoord[0][1],
	//					t->texcoord[1][1],
	//					t->texcoord[2][1]);

	if (u > 1.0 || v > 1.0 || w > 1.0) {
		Log("uvw: %lf %lf %lf\n",u,v,w);
	}
	//Log("sf->tm->yres: %i %i\n",sf->tm->yres,sf->tm->xres);

	Flt t1[2], t2[2];
	t1[0] = t->texcoord[1][0] - t->texcoord[0][0];
	t1[1] = t->texcoord[1][1] - t->texcoord[0][1];
	t2[0] = t->texcoord[2][0] - t->texcoord[0][0];
	t2[1] = t->texcoord[2][1] - t->texcoord[0][1];
	ttx[0] = t->texcoord[0][0] + t1[0]*u + t2[0]*v;
	ttx[1] = t->texcoord[0][1] + t1[1]*u + t2[1]*v;

	//assert...
	//if (ttx[0] < 0.0) {
	//	Log("ttx[0]: %lf\n",ttx[0]);
	//	ttx[0] = 0.0;
	//}
	//if (ttx[0] > 1.0) {
	//	Log("ttx[0]: %lf\n",ttx[0]);
	//	ttx[0] = 1.0;
	//}
	//if (ttx[1] < 0.0) {
	//	Log("ttx[1]: %lf\n",ttx[1]);
	//	ttx[1] = 0.0;
	//}
	//if (ttx[1] > 1.0) {
	//	Log("ttx[1]: %lf\n",ttx[1]);
	//	ttx[1] = 1.0;
	//}

	int row = (int)(ttx[1]*(Flt)sf->tm->yres);
	int col = (int)(ttx[0]*(Flt)sf->tm->xres);
	//This allows tiling of the texture
	j = sf->tm->yres;
	while (row >= j)
		row -= j;
	j = sf->tm->xres;
	while (col >= j)
		col -= j;
	//
	//Get the offset into the texture data
	//unsigned char *ptr = sf->tm->data + (row * sf->tm->xres * 3 + col * 3);
	unsigned char *ptr = sf->tm->data + (row * sf->tm->xres + col)*3;
	//Log("*ptr: %i %i %i\n",*ptr,*(ptr+1), *(ptr+2));
	hit->color[0] = (Flt)*(ptr  ) / 256.0;
	hit->color[1] = (Flt)*(ptr+1) / 256.0;
	hit->color[2] = (Flt)*(ptr+2) / 256.0;
}




