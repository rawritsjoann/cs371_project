//cs371 Introduction To Computer Graphics
//program:      atrace.cpp
//author:       Gordon Griesel
//date written: Summer 2013
//              This program was written by Gordon Griesel for use by
//              the students of cs371 Fall 2013/14/15
//purpose:      a framework for ray tracing
//
//Originally written under Linux 12.10 with following versions:
//
//Linux    Xorg     OpenGL
//-------  -------  ---------------
//12.10    1.13.0   3.0 Mesa  9.0.3   <---works
//14.10    1.15.1   3.0 Mesa 10.1.3   <---works with double buffer on
//
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <GL/glx.h>
#include "defs.h"
#include "log.h"
#include "fonts.h"
#include "vector.h"
#include "extern.h"
//for Bounding volume hierarchy
#include "bvh.h"

//XWindows globals.
Display *dpy;
Window win;
//This project has a few global variables, prefixed with "g.".
Global g;


int main(int argc, char *argv[])
{
	XEvent e;
	//int fullscreen=0;
	if (argc > 1) {
		if (*argv[1]) {}
		//Any command-line parameter could causes full screen OpenGL.
		//fullscreen=1;
	}
	logOpen();
	initXwindow();
	initOpengl();
	init();
	g.mode = MODE_HELP;
	//
	while (!g.done) {
		//handle all events in queue...
		while (XPending(dpy)) {
			XNextEvent(dpy, &e);
			checkResize(&e);
			checkMouse(&e);
			checkKeys(&e);
			//break;
		}
		if (g.updateScreen || g.mode) {
			render();
			g.updateScreen=0;
			glXSwapBuffers(dpy, win);
			//XFlush(dpy);
		}
	}
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
	cleanup();
	logClose();
	exit(EXIT_SUCCESS);
}

void checkResize(XEvent *e) {
	//ConfigureNotify or Expose is sent when the window is resized.
	if (e->type != Expose)
		return;
	//XConfigureEvent xce = e->xconfigure;
	//g.screen.xres = xce.width;
	//g.screen.yres = xce.height;
	//printf("%i %i\n",g.screen.xres,g.screen.yres);
	//reshapeWindow(g.screen.xres, g.screen.yres);
	XWindowAttributes wa;
	XGetWindowAttributes(dpy, win, &wa);
	//XWindowAttributes *window_attributes_return;
	//printf("e->type: %i  %i %i\n",e->type, wa.width, wa.height);
	if (wa.width != g.screen.xres || wa.height != g.screen.yres) {
		reshapeWindow(wa.width, wa.height);
	}
}

double timediff(struct timespec *start, struct timespec *end)
{
	double diff = (double)(end->tv_sec - start->tv_sec ) +
	              (double)(end->tv_nsec - start->tv_nsec) / 1.0e9;
	return diff;
}

inline void setpixel(const int x, const int y, Vec rgb)
{
	//Log("setpixel(%i %i : %u %u %u)...\n",x,y,rgb[0],rgb[1],rgb[2]);
	int offset = y * g.screen.xres3 + x * 3;
	*(g.screendata+offset  ) = (float)rgb[0];
	*(g.screendata+offset+1) = (float)rgb[1];
	*(g.screendata+offset+2) = (float)rgb[2];
}

#if defined(USE_BVH_FOR_SHADOWS) && defined(USE_SHADOW_CACHING)
void shadowBvh(BVH *bvh, Ray *ray, Flt disttolight, Light *l)
{
	//The ray goes toward a light.
	//Does the ray intersect any objects along the way?
	//If so, the ray origin is in shadow.
	//Distance to hit must be shorter than distance to light.
	int ret;
	if (!bvh) return;
	ret = checkSlabs(&bvh->bb, ray);
	if (ret) {
		Hit hit;
		Object *o = bvh->ohead;
		while (o) {
			if (o->intersectFunc(o, ray, &hit)) {
				if (hit.t < disttolight) {
					l->cacheObj=o;
					return;
				}
			}
			o = o->next;
		}
		//Traverse the tree branches...
		shadowBvh(bvh->left,  ray, disttolight, l);
		shadowBvh(bvh->right, ray, disttolight, l);
	}
	return;
}
#endif

#if defined(USE_BVH_FOR_SHADOWS) && !defined(USE_SHADOW_CACHING)
void shadowBvh(BVH *bvh, Ray *ray, Flt disttolight, int *shad)
{
	int ret;
	if (!bvh) return;
	ret = checkSlabs(&bvh->bb, ray);
	if (ret) {
		Hit hit;
		Object *o = bvh->ohead;
		while (o) {
			if (o->intersectFunc(o, ray, &hit)) {
				if (hit.t < disttolight) {
					*shad=1;
					return;
				}
			}
			o = o->next;
		}
		//Traverse the tree branches...
		shadowBvh(bvh->left,  ray, disttolight, shad);
		shadowBvh(bvh->right, ray, disttolight, shad);
	}
	return;
}
#endif

#if defined(USE_SHADOW_CACHING)
int testShadowCache(Ray *ray, Flt disttolight, Object *o)
{
	//Shadow caching is based on the principle of locality.
	//Two types of locality could be in play here...
	//     .spacial locality
	//     .sequential locality
	//
	Hit hit;
	if (o->intersectFunc(o, ray, &hit)) {
		//important: distance to hit must be shorter than distance to light.
		//if (hit.t < disttolight) return 1;
		return (hit.t < disttolight);
	}
	return 0;
}

int traceShadow(Ray *ray, Flt disttolight, Light *l)
{
	//The ray goes toward a light
	//Does the ray intersect any objects along the way?
	//If so, the ray origin is in shadow
	//Distance to hit must be shorter than distance to light.
	#ifdef USE_BVH_FOR_SHADOWS
		shadowBvh(g.bvhHead, ray, disttolight, l);
		return (l->cache_obj!=NULL);
	#else //USE_BVH_FOR_SHADOWS
		Hit hit;
		Object *o = g.objectHead;
		while (o) {
			if (o->intersectFunc(o, ray, &hit)) {
				if (hit.t < disttolight) {
					l->cache_obj = o;
					return 1;
				}
			}
			o = o->next;
		}
		return 0;
	#endif //USE_BVH_FOR_SHADOWS
}
#endif //USE_SHADOW_CACHING

#if !defined(USE_SHADOW_CACHING)
int traceShadow(Ray *ray, Flt disttolight)
{
	//No shadow caching, so fewer parameters are needed.
	//Distance to hit must be shorter than distance to light.
	#ifdef USE_BVH_FOR_SHADOWS
		int shad=0;
		shadowBvh(g.bvhHead, ray, disttolight, &shad);
		return shad;
	#else //USE_BVH_FOR_SHADOWS
		Hit hit;
		Object *o = g.objectHead;
		while (o) {
			if (o->intersectFunc(o, ray, &hit)) {
				if (hit.t < disttolight)
					return 1;
			}
			o = o->next;
		}
		return 0;
	#endif //USE_BVH_FOR_SHADOWS
}
#endif

int get_light_contribution(Vec p, Vec nhit, Vec rgb)
{
	//Start vectors at the hit spot p and shoot ray to lights.
	int k;
	Flt dot, disttolight, len, fs;
	Vec v, v0, tweak, accum;
	Ray rx;
	VecZero(rgb);
	Light *l = g.lightHead;
	while (l) {
		VecSub(l->center, p, v);
		disttolight = VecNormalizeL(v);
		dot = VecDot(v,nhit);
		//Check only if normal is toward light
		if (dot > 0.0) {
			if (l->area) {
				//It's an area light, so cast a bunch of rays
				//We will use a monte carlo method
				//Accum the hit/no-hit stats
				VecZero(accum);
				len = l->radius / disttolight;
				for (k=0; k<l->nsamples; k++) {
					//tweak ray direction
					MakeVector(0.5-rnd(), 0.5-rnd(), 0.5-rnd(), tweak);
					VecNormalize(tweak);
					VecAddS(len, tweak, v, v0);
					VecCopy(p, rx.o);
					//go in direction to light source
					VecCopy(v0, rx.d);
					nudgeRayForward(&rx);
					#ifdef USE_SHADOW_CACHING
						l->cache_obj=NULL;
						if (traceShadow(&rx, disttolight, l))
							continue;
					#else //USE_SHADOW_CACHING
						if (traceShadow(&rx, disttolight))
							continue;
					#endif //USE_SHADOW_CACHING

					//no penumbra
					//VecAdd(l->color, accum, accum);

					//This blends colors well, by getting a new dot each time
					//a ray is sent to the light source.
					dot = VecDot(v0, nhit);
					accum[0] += l->color[0] * dot;
					accum[1] += l->color[1] * dot;
					accum[2] += l->color[2] * dot;
				}
				fs = 1.0 / (Flt)l->nsamples;
				//VecS(fs, accum, accum);
				//VecAdd(rgb, accum, rgb);
				rgb[0] += accum[0] * fs;
				rgb[1] += accum[1] * fs;
				rgb[2] += accum[2] * fs;

				goto nextcontlight;
			}
			//Not an area light...
			VecCopy(p, rx.o);
			//go in direction to light source
			VecCopy(v, rx.d);
			nudgeRayForward(&rx);
			#ifdef USE_SHADOW_CACHING
				if (l->cache_obj) {
					if (test_shadow_cache(&rx, disttolight, l->cache_obj)) {
						//Log("cache hit\n");
						goto nextcontlight;
					}
				}
				l->cache_obj=NULL;
				if (traceShadow(&rx, disttolight, l)) {
					goto nextcontlight;
				}
			#else //USE_SHADOW_CACHING
				if (traceShadow(&rx, disttolight)) {
					goto nextcontlight;
				}
			#endif //USE_SHADOW_CACHING
		}
		dot = (dot < 0.0) ? 0.0 : dot;
		rgb[0] += l->color[0] * dot;
		rgb[1] += l->color[1] * dot;
		rgb[2] += l->color[2] * dot;
		//This function may skip right to here
		nextcontlight:
		//Move to the next light source
		l = l->next;
	}
	return 1;
}

inline Flt getFalloffFactor(Light *l, Flt disttolight)
{
	switch (l->falloff) {
		//dimmer if farther from the light
		case 1:
			return (l->falloffVal - disttolight) * l->oofalloffVal;
		case 2:
			return (l->falloffVal - (disttolight*disttolight)) *
			      l->oofalloffVal;
	}
	return 1.0;
}

void mediaGetLightContrib(Vec p, Vec rgb)
{
	//Start vectors at the hit spot p and check if this spot is in shadow.
	int k;
	Flt disttolight, len, ns, fs;
	Vec v, v0, tweak, accum;
	Ray rx;
	VecZero(rgb);
	Light *l = g.lightHead;
	while (l) {
		VecSub(l->center, p, v);
		disttolight = VecNormalizeL(v);
		if (l->area) {
			//It's an area light, so cast a bunch of rays.
			//We will use a monte carlo method.
			//Accum the hit/no-hit stats.
			VecZero(accum);
			len = l->radius / disttolight;
			for (k=0; k<l->nsamples; k++) {
				//tweak ray direction
				MakeVector(0.5-rnd(), 0.5-rnd(), 0.5-rnd(), tweak);
				VecNormalize(tweak);
				VecAddS(len, tweak, v, v0);
				VecCopy(p, rx.o);
				//go in direction to light source
				VecCopy(v0, rx.d);
				//nudgeRayForward(&rx);
				if (traceShadow(&rx, disttolight))
					continue;
				accum[0] += l->color[0];
				accum[1] += l->color[1];
				accum[2] += l->color[2];
			}
			ns = 1.0 / (Flt)l->nsamples;
			VecS(ns, accum, accum);
			fs = getFalloffFactor(l, disttolight);
			if (fs > 0.0) {
				rgb[0] += accum[0] * fs;
				rgb[1] += accum[1] * fs;
				rgb[2] += accum[2] * fs;
			}
			goto nextcontlight;
		}
		//Not an area light...
		VecCopy(p, rx.o);
		//go in direction to light source
		VecCopy(v, rx.d);
		if (traceShadow(&rx, disttolight)) {
			goto nextcontlight;
		}
		fs = getFalloffFactor(l, disttolight);
		if (fs > 0.0) {
			rgb[0] += l->color[0] * fs;
			rgb[1] += l->color[1] * fs;
			rgb[2] += l->color[2] * fs;
		}
		//This function may skip right to here
		nextcontlight:
		//Move to the next light source
		l = l->next;
	}
	return;
}

void reflect(const Vec I, const Vec N, Vec R)
{
	//I = incident vector
	//N = the surface normal
	//R = reflected ray
	Flt dot = -VecDot(I, N);
	Flt len = 2.0 * dot;
	R[0] = len * N[0] + I[0];
	R[1] = len * N[1] + I[1];
	R[2] = len * N[2] + I[2];
}

int refract(const Flt eta, const Vec I, const Vec N, Vec T)
{
	//Calculates direction of refracted angle, T.
	//Returns 0 if total internal reflection occurs.
	//eta = ratio of old/new iors
	//    = relative index-of-refraction
	//I   = incident vector
	//N   = surface normal
	//T   = transmitted vector (calculated)
	Flt dot, c1, cs2;
	if (eta == 1.0) {
		VecCopy(I, T);
		return 1;
	}
	dot = -VecDot(I, N);
	c1 = dot;
	cs2 = 1.0 - eta*eta * (1.0 - c1*c1);
	if (cs2 < 0.0) {
		//Total internal reflection
		//Reflect off surface instead of refract
		reflect(I, N, T);
		return 0;
	}
	cs2 = eta*c1 - sqrt(cs2);
	//VecComb(eta, I, cs2, N, T);
	//combine the vectors...
	T[0] = eta * I[0] + cs2 * N[0];
	T[1] = eta * I[1] + cs2 * N[1];
	T[2] = eta * I[2] + cs2 * N[2];
	return 1;
}


#ifdef USE_BVH_FOR_TRACING
Object *check_hierarchy(BVH *bvh, Ray *ray, Hit *closehit, Object *closeObj)
{
	//Look for intersection of ray with an object.
	int ret;
	Hit hit;
	Object *o;
	if (!bvh) return closeObj;
	ret = checkSlabs(&bvh->bb, ray);
	if (ret) {
		o = bvh->ohead;
		while (o) {
			if (o->intersectFunc(o, ray, &hit)) {
				if (hit.t < closehit->t) {
					closehit->t = hit.t;
					VecCopy(hit.p, closehit->p);
					closeObj = o;
				}
			}
			o = o->next;
		}
		//Traverse the tree branches...
		closeObj = check_hierarchy(bvh->left,  ray, closehit, closeObj);
		closeObj = check_hierarchy(bvh->right, ray, closehit, closeObj);
	}
	return closeObj;
}

#else //USE_BVH_FOR_TRACING

Object *check_sequential(Ray *ray, Hit *closehit, Object *closeObj)
{
	//Check for intersection of ray with an object. Each object struct has
	//a function pointer that leads us to the intersection routine.
	Hit hit;
	Object *o = g.objectHead;
	while (o) {
		if (o->intersectFunc(o, ray, &hit)) {
			if (hit.t < closehit->t) {
				closehit->t = hit.t;
				VecCopy(hit.p, closehit->p);
				closeObj = o;
			}
		}
		o = o->next;
	}
	return closeObj;
}
#endif //USE_BVH_FOR_TRACING


inline void jitter(Vec dir)
{
	dir[0] += (rnd() * g.studio.jval - g.studio.jval2);
	dir[1] += (rnd() * g.studio.jval - g.studio.jval2);
}

Flt getAmbientOcclusion(Hit *hit, const Flt dist)
{
	//Returns portion of ambient light reaching this point.
	//
	//Send out some random rays from hit point.
	//No hits    = full ambient light
	//More hits  = less light
	//Close hits = less light

	//to be coded by students



	//this code is to suppress compiler warnings.
	hit->t = hit->t;
	if (dist) {}
	return 0.0;
}

void trace(Ray *ray, Vec rgb, Flt weight, int level, int trans)
{
	//This is the core of our ray-tracer
	//Cast rays, then recurse as the ray bounces around
	int j, rflag=0;
	Flt dot;
	Vec nhit, v, reflection;
	Vec surfaceColor, specularColor, transmittedColor;
	Vec lcont, nh;
	Vec fresnel;
	Hit closehit;
	Ray rx;
	Object *closeObj=NULL;
	Surface *sf=NULL;
	static Surface *lastsf=NULL;
	closehit.t = 9e25;
	//Log("trace()...\n");
	//BVH is bounding volume hierarchy.
	#ifdef USE_BVH_FOR_TRACING
		closeObj = check_hierarchy(g.bvhHead, ray, &closehit, closeObj);
	#else //USE_BVH_FOR_TRACING
		closeObj = check_sequential(ray, &closehit, closeObj);
	#endif //USE_BVH_FOR_TRACING
	if (closeObj==NULL ||
		level > g.studio.maxDepth ||
		weight < g.studio.minWeight) {
		// - ray did not hit an object
		// - ray has bounced around too many times
		// - ray has gotten very dim, so
		//send it to the background.
		Vec col;
		//background_hit(ray->d, col);
		backgroundHit(ray, col);
		VecAddS(weight, col, rgb, rgb);
		return;
	}
	//get surface characteristics
	sf = (Surface *)closeObj->surface;
	VecCopy(sf->diffuse, closehit.color);
	//Is there a pattern?
	switch(sf->pattern) {
		case 1: checkerPattern(closehit.p, sf, closehit.color); break;
		case 2: perlin_texture_2D(closehit.p, sf, closehit.color); break;
		case 3: perlin_texture_3D(closehit.p, sf, closehit.color); break;
	}
	closeObj->normalFunc(closeObj, &closehit, nhit);
	VecNormalize(nhit);
	if (sf->insideout) {
		VecNegate(nhit);
	}

	//this can be optimized
	VecCopy(nhit, closehit.nhit);
	//
	//Is there a texture map?
	if (sf->tm != NULL) {
		resolveTexmap(closeObj, &closehit);
	}
	//
	get_light_contribution(closehit.p, nhit, lcont);
	if (g.studio.ambientOcclusionN) {
		Flt aov;
		aov = getAmbientOcclusion(&closehit, g.studio.ambientOcclusionDist);
		surfaceColor[0] = closehit.color[0]*
		                  (lcont[0]+g.studio.ambient[0]*aov)*weight;
		surfaceColor[1] = closehit.color[1]*
		                  (lcont[1]+g.studio.ambient[1]*aov)*weight;
		surfaceColor[2] = closehit.color[2]*
		                  (lcont[2]+g.studio.ambient[2]*aov)*weight;
	} else {
		surfaceColor[0] = closehit.color[0]*
		                  (lcont[0]+g.studio.ambient[0])*weight;
		surfaceColor[1] = closehit.color[1]*
		                  (lcont[1]+g.studio.ambient[1])*weight;
		surfaceColor[2] = closehit.color[2]*
		                  (lcont[2]+g.studio.ambient[2])*weight;
	}
	if (sf->emitYN) {
		//This surface is actually emiting some light
		for (j=0; j<3; j++) {
			surfaceColor[j] += sf->emit[j] * weight;
		}
	}
	//if (g.nphotons) {
	//	Vec plight;
	//	getPhotonContribution(&closehit, nhit, plight);
	//	surfaceColor[0] += plight[0] * weight;
	//	surfaceColor[1] += plight[1] * weight;
	//	surfaceColor[2] += plight[2] * weight;
	//}
	if (trans) {
		//What does (trans != 0) mean?
		//Current ray is traveling through a transparent substance.
		//The medium could block some of the light and color.
		//Subtract any color contained in the medium.
		//Amount is based on length of ray through medium.
		if (lastsf->transmitYN == 2) {
			//lastsf is the last surface we hit before the current one.
			surfaceColor[0] -= lastsf->transmit[0] * weight * closehit.t;
			surfaceColor[1] -= lastsf->transmit[1] * weight * closehit.t;
			surfaceColor[2] -= lastsf->transmit[2] * weight * closehit.t;
		}
	}
	//
	//
	if (sf->specularYN) {
		//A reflective surface was hit
		VecCopyNeg(ray->d, v);
		VecZero(specularColor);
		reflect(ray->d, nhit, reflection);
		//rflag says that a reflection vector has been calculated
		rflag=1;
		VecCopy(reflection, rx.d);
		VecCopy(closehit.p, rx.o);
		nudgeRayForward(&rx);
		trace(&rx, specularColor, weight*0.5, level+1, 0);
		dot=VecDot(nhit,v);
		for (j=0; j<3; j++) {
			//experiment with these equations...
			//Flt spec = (sf->specular[0]+sf->specular[1]+sf->specular[2])/3.0;
			//fresnel = spec + (1.0-spec)*spec*(1.0-(pow(dot,3.0)));
			//fresnel = spec + (1.0-spec)*(1.0-(pow(dot,3.0)));
			//fresnel = specularColor[j] +
			//						specularColor[j]*(1.0-(pow(dot,5.0)));
			fresnel[j] = sf->specular[j] +
									sf->specular[j]*(1.0-(pow(dot,5.0)));
			surfaceColor[j] += specularColor[j] * fresnel[j] * weight;
		}
	}
	//
	//
	if (sf->transmitYN) {
		//transparent surface
		int entering = 1;
		Flt eta = sf->iorOutside / sf->iorInside;
		lastsf = sf;
		VecCopy(nhit, nh);
		if ((VecDot(ray->d, nh)) > 0.0) {
			//Normal is pointing the wrong way, so flip.
			//Maybe a transparent shell. No big deal.
			VecNegate(nh);
			eta = 1.0 / eta;
			entering = 0;
		}
		//T is the transmitted vector
		//refract(eta, ray->d, nh, T);
		//VecCopy(T, rx.d);
		refract(eta, ray->d, nh, rx.d);
		//rx.d is the direction of the transmitted vector
		//Log("refracted ray: %lf %lf %lf\n",rx.d[0],rx.d[1],rx.d[2]);
		VecCopy(closehit.p, rx.o);
		VecZero(transmittedColor);
		nudgeRayForward(&rx);
		if (sf->translucent.YN && entering) {
			//Surface has translucency, so shoot a bunch of tweaked
			//rays and average the results, to get a bluring effect.
			int k;
			Flt tval  = sf->translucent.val;
			Flt tval2 = sf->translucent.val*0.5;
			Vec totcol={0,0,0};
			Vec savedir;
			VecCopy(rx.d, savedir);
			for (k=0; k<sf->translucent.nsamples; k++) {
				//tweak ray by tval
				VecCopy(savedir, rx.d);
				rx.d[0] += (rnd()*tval-tval2);
				rx.d[1] += (rnd()*tval-tval2);
				rx.d[2] += (rnd()*tval-tval2);
				VecNormalize(rx.d);
				VecZero(transmittedColor);
				trace(&rx, transmittedColor, weight, level+1, entering);
				totcol[0] += transmittedColor[0];
				totcol[1] += transmittedColor[1];
				totcol[2] += transmittedColor[2];
			}
			Flt div = 1.0 / (Flt)sf->translucent.nsamples;
			VecS(div, totcol, totcol);
			surfaceColor[0] += totcol[0];
			surfaceColor[1] += totcol[1];
			surfaceColor[2] += totcol[2];
		} else {
			trace(&rx, transmittedColor, weight, level+1, entering);
			surfaceColor[0] += transmittedColor[0];
			surfaceColor[1] += transmittedColor[1];
			surfaceColor[2] += transmittedColor[2];
		}
	}
	//
	//Check for specular highlighting
	if (sf->highlightYN) {
		Flt disttolight, high;
		if (!rflag) {
			//reflection vector is not calculated yet
			reflect(ray->d, nhit, reflection);
		}
		//Shoot a ray to each light
		Light *l = g.lightHead;
		while (l) {
			if (!l->specular) {
				goto nextshadowlight;
			}
			VecSub(l->center, closehit.p, v);
			disttolight = VecNormalizeL(v);
			//v = vector from hit to light
			dot = VecDot(v,nhit);
			//Only if dot is toward light.
			if (dot > 0.0) {
				//Is spot in shadow?
				VecCopy(closehit.p, rx.o);
				VecCopy(v, rx.d);
				nudgeRayForward(&rx);
				#ifdef USE_SHADOW_CACHING
				if (l->cache_obj) {
					if (test_shadow_cache(&rx, disttolight, l->cache_obj)) {
						//Log("cache hit\n");
						goto nextshadowlight;
					}
				}
				l->cache_obj=NULL;
				if (traceShadow(&rx, disttolight, l)) {
					goto nextshadowlight;
				}
				#else //USE_SHADOW_CACHING
				if (traceShadow(&rx, disttolight)) {
					goto nextshadowlight;
				}
				#endif //USE_SHADOW_CACHING
			}
			//specular highlight...
			high = VecDot(reflection, v);
			if (high > 0.0) {
				high = pow(high, sf->spot);
				//VecAddS(high, sf->highlight, surfaceColor, surfaceColor);
				surfaceColor[0] += high * sf->highlight[0];
				surfaceColor[1] += high * sf->highlight[1];
				surfaceColor[2] += high * sf->highlight[2];
			}
			nextshadowlight:
			l = l->next;
		}
	}
	if (g.studio.hazeOn) {
		extern void testForHaze(Ray *ray, Flt t, Vec col);
		testForHaze(ray, closehit.t, surfaceColor);
	}
	//VecAdd(rgb, surfaceColor, rgb);
	rgb[0] += surfaceColor[0];
	rgb[1] += surfaceColor[1];
	rgb[2] += surfaceColor[2];
	return;
}

void aperture(Ray *ray)
{
	//Not a pinhole camera
	//Make v a vector of length: focal length
	Vec v;
	VecAddS(g.studio.aperture.focalLength, ray->d, ray->o, v);
	//Tweak the aperture end (origin) of the vector.
	//All tweaked rays meet at the focal length.
	ray->o[0] += (rnd()*g.studio.aperture.radius - g.studio.aperture.r5);
	ray->o[1] += (rnd()*g.studio.aperture.radius - g.studio.aperture.r5);
	ray->o[2] += (rnd()*g.studio.aperture.radius - g.studio.aperture.r5);
	//Make a new vector
	VecSub(v, ray->o, ray->d);
	VecNormalize(ray->d);
	//Log("ray.dir: %lf %lf %lf\n",ray.dir[0],ray.dir[1],ray.dir[2]);
}

void cast_aperture_rays(Ray *ray, Vec out, Vec savedir, Vec rgb)
{
	int i;
	Ray saver;
	//printf("cast_aperture_rays()...\n");
	VecZero(rgb);
	VecAdd(savedir, out, saver.d);
	VecNormalize(saver.d);
	VecCopy(ray->o, saver.o);
	for (i=0; i<g.studio.aperture.nsamples; i++) {
		VecCopy(saver.o, ray->o);
		VecCopy(saver.d, ray->d);
		aperture(ray);
		trace(ray, rgb, 1.0, 1, 0);
	}
	Flt div = 1.0 / (Flt)g.studio.aperture.nsamples;
	VecS(div, rgb, rgb);
}

void cast_aa_aperture_rays(Ray *ray, Vec out, Vec savedir, Vec rgb, Flt *fs)
{
	//Antialiasing is added to this function.
	//Tweak the ray before adjusting for aperture size.
	int i;
	int n = g.studio.aperture.nsamples * g.studio.nsamples;
	Vec v;
	Ray saver;
	Flt s0 = fs[0];
	Flt s1 = fs[1];
	Flt s02 = s0*0.5;
	Flt s12 = s1*0.5;
	VecCopy(ray->o, saver.o);
	v[2] = savedir[2];
	for (i=0; i<n; i++) {
		v[0] = savedir[0] + (rnd()*s0-s02);
		v[1] = savedir[1] + (rnd()*s1-s12);
		VecAdd(v, out, ray->d);
		VecCopy(saver.o, ray->o);
		aperture(ray);
		trace(ray, rgb, 1.0, 1, 0);
	}
	Flt div = 1.0 / (Flt)n;
	VecS(div, rgb, rgb);
}

void cast_distribution(Ray *ray, Vec out, Vec savedir, Vec rgb, Flt *fs)
{
	//shoot 4 more rays
	Vec v;
	Flt xx = fs[0]*0.5;
	Flt yy = fs[1]*0.5;
	VecCopy(savedir,v);
	v[0] += xx;
	v[1] += yy;
	if (g.studio.jitter) jitter(v);
	VecAdd(v, out, ray->d);
	VecNormalize(ray->d);
	trace(ray, rgb, 1.0, 1, 0);
	VecCopy(savedir,v);
	v[0] -= xx;
	v[1] += yy;
	if (g.studio.jitter) jitter(v);
	VecAdd(v, out, ray->d);
	VecNormalize(ray->d);
	trace(ray, rgb, 1.0, 1, 0);
	VecCopy(savedir,v);
	v[0] += xx;
	v[1] -= yy;
	if (g.studio.jitter) jitter(v);
	VecAdd(v, out, ray->d);
	VecNormalize(ray->d);
	trace(ray, rgb, 1.0, 1, 0);
	VecCopy(savedir,v);
	v[0] -= xx;
	v[1] -= yy;
	if (g.studio.jitter) jitter(v);
	VecAdd(v, out, ray->d);
	VecNormalize(ray->d);
	trace(ray, rgb, 1.0, 1, 0);
	VecS(0.2, rgb, rgb);
}

void cast_montecarlo_rays(Ray *ray,
                          Vec out, Vec savedir, Vec rgb, Flt *fs, int dim)
{
	//This is a type of distributed ray-tracing.
	int i;
	Vec dir;
	Flt s02 = fs[0];
	Flt s12 = fs[1];
	Flt s0 = s02*0.5;
	Flt s1 = s12*0.5;
	if (dim > 1) {
		s02 *= (Flt)dim;
		s12 *= (Flt)dim;
		s0 *= s02*0.5;
		s1 *= s12*0.5;
	}
	VecZero(rgb);
	dir[2] = savedir[2];
	//ray->d[2] = dir[2] + out[2];
	//shoot multiple rays
	for (i=0; i<g.studio.nsamples; i++) {
		//VecCopy(savedir, dir);
		dir[0] = savedir[0];
		dir[1] = savedir[1];
		if (g.studio.jitter)
			jitter(dir);
		ray->d[0] = dir[0] + (rnd()*s02-s0) + out[0];
		ray->d[1] = dir[1] + (rnd()*s12-s1) + out[1];
		ray->d[2] = dir[2] + out[2];
		VecNormalize(ray->d);
		trace(ray, rgb, 1.0, 1, 0);
	}
	Flt div = 1.0 / (Flt)g.studio.nsamples;
	VecS(div, rgb, rgb);
}

void cast_montecarloA_rays(Ray *ray,
                           Vec out, Vec savedir, Vec rgb, Flt *fs, int dim)
{
	//This is a type of distributed ray-tracing.
	//It is an adaptive technique, because if the colors are dissimilar,
	//then more tracing is done.
	int i, n=0, again=0;
	Vec dir;
	Flt div;
	Flt s02 = fs[0];
	Flt s12 = fs[1];
	Flt s0 = fs[0]*0.5;
	Flt s1 = fs[1]*0.5;
	if (dim > 1) {
		s02 *= (Flt)dim;
		s12 *= (Flt)dim;
		s0 *= (Flt)dim*0.5;
		s1 *= (Flt)dim*0.5;
	}
	Vec diff, rgb1, rgb2={0,0,0};
	dir[2] = savedir[2];
	VecZero(rgb);
	for (i=0; i<g.studio.nsamples; i++) {
		dir[0] = savedir[0];
		dir[1] = savedir[1];
		if (g.studio.jitter)
			jitter(dir);
		ray->d[0] = dir[0] + (rnd()*s02-s0) + out[0];
		ray->d[1] = dir[1] + (rnd()*s12-s1) + out[1];
		ray->d[2] = dir[2] + out[2];
		VecNormalize(ray->d);
		VecZero(rgb1);
		trace(ray, rgb1, 1.0, 1, 0);
		VecAdd(rgb, rgb1, rgb);
		if (i) {
			//Is there a significant difference between the
			//last two pixel colors?
			VecSub(rgb1,rgb2,diff);
			VecABS(diff);
			if (diff[0] + diff[1] + diff[2] > g.studio.var)
				again++;
		}
		VecCopy(rgb1, rgb2);
	}
	n += g.studio.nsamples;
	//If significant difference was found between colors,
	//								then get some more points.
	//for (i=0; i<g.studio.nsamples*again; i++) {
	again <<= 1;
	for (i=0; i<again; i++) {
		dir[0] = savedir[0];
		dir[1] = savedir[1];
		if (g.studio.jitter)
			jitter(dir);
		ray->d[0] = dir[0] + (rnd()*s02-s0) + out[0];
		ray->d[1] = dir[1] + (rnd()*s12-s1) + out[1];
		ray->d[2] = dir[2] + out[2];
		VecNormalize(ray->d);
		VecZero(rgb1);
		trace(ray, rgb1, 1.0, 1, 0);
		VecAdd(rgb, rgb1, rgb);
	}
	//n += g.studio.nsamples*again;
	n += again;
	div = 1.0 / (Flt)n;
	VecS(div, rgb, rgb);
}


void castRays(const int dim)
{
	//dim is the granular sample size in pixels.
	//dim of 1 will sample each pixel.
	//dim of 2 will sample each 2x2 area as a pixel.
	//This is used to do quick traces of complex scenes.
	int i,j,k,m;
	int dim2 = dim/2;
	int px = dim2;
	int py = dim2;
	Flt ty = 1.0 / (g.screen.fyres - 1.0);
	Flt tx = 1.0 / (g.screen.fxres - 1.0);
	Flt viewAnglex, aspectRatio;
	Flt frustumheight, frustumwidth, frustumstep[2];
	Vec rgb, eye, dir, left, up, out, savedir;
	Ray ray;
	struct timespec timerstart, timerend;
	int save1=0;
	Flt save2=0.0;
	unsigned int save3=0;

	//Log("castRays(%i)...\n",dim);
	//printf("castRays(%i)...\n",dim); fflush(stdout);
	//If a trace is already running, leave.
	if (g.tracing)
		return;
	//If mouse rectangle was dragged, use present scene.
	if (!g.rFlag) {
		//Initialize object structures, etc.
		cleanupLists();
		//read the scene definition file
		if (!readScene()) {
			printf("ERROR: no scene files were found!\n"); fflush(stdout);
			exit(EXIT_FAILURE);
		}
	}
	//Check here for an animation running. We need to process the scene file,
	//but maybe not draw the scene.
	if (g.anim.running && g.anim.nframes) {
		//printf("g.anim.running\n"); fflush(stdout);
		if (g.anim.skip)
			return;
		g.anim.frameRendered=1;
	}
	VecNormalize(g.studio.up);
	VecSub(g.studio.at, g.studio.from, out);
	VecNormalize(out);
	aspectRatio = g.screen.fxres / g.screen.fyres;
	#if defined(USE_BVH_FOR_TRACING) || \
		defined(USE_BVH_FOR_SHADOWS)
		buildHierarchy();
	#endif
	//
	//Our scene is in place.
	//
	if (g.aaOn) {
		//printf("NOTICE --> g.aaOn\n");
		save1 = g.studio.nsamples;
		g.studio.nsamples = g.aaCount;
		save2 = g.studio.var;
		g.studio.var = g.aaTolerance;
		g.studio.nsamples = save1;
		g.studio.var = save2;
		save3 = g.studio.shoot;
		g.studio.shoot |= SHOOT_MONTECARLO;
		g.studio.shoot |= SHOOT_ADAPTIVE;
	}
	//
	extern void setup_haze(void);
	setup_haze();
	//
	//-------------------------------------------------------------------------
	//If there is a photon light, build a photon map.
	//if (g.nphotons)
	//	buildPhotonMap();
	//-------------------------------------------------------------------------
	viewAnglex = degreesToRadians(g.studio.angle*0.5);
	frustumwidth = tan(viewAnglex);
	frustumheight = frustumwidth / aspectRatio;
	//frustumwidth is actually half the distance across screen
	frustumstep[0] = (frustumwidth  * 2.0) / g.screen.fxres;
	frustumstep[1] = (frustumheight * 2.0) / g.screen.fyres;
	//compute the left and up vectors...
	VecCross(out, g.studio.up, left);
	VecNormalize(left);
	VecCross(out, left, up);
	//
	VecCopy(g.studio.from, eye);
	g.tracing=1;
	//Trace every pixel...
	int istart = 0;
	int iend = g.screen.yres;
	int jstart = 0;
	int jend = g.screen.xres;
	//... unless a rectangle was just dragged.
	if (g.rFlag) {
		jstart = g.r1[0];
		istart = g.r1[3];
		jend   = g.r1[2];
		iend   = g.r1[1]+1;
		g.rFlag=0;
	} else {
		//if (g.timeEstimation) {
		//	g.tracing=0;
		//	return;
		//}
	}
	clock_gettime(CLOCK_REALTIME, &timerstart);
	int interval=0;
	//printf("start loop\n");
	for (i=istart; i<iend; i+=dim) {
		py = i + dim2;
		if (i < g.studio.row_start) {
			//this is used when rendering a very complex scene in pieces.
			glXSwapBuffers(dpy, win);
			continue;
		}
		for (j=jstart; j<jend; j+=dim) {
			px = j + dim2;
			//Start the color at black
			VecZero(rgb);
			//Start the ray origin at the eye
			VecCopy(eye, ray.o);
			//
			//#define EXPANDED_CALCULATION
			//#define COMPRESSED_CALCULATION
			#define OPTIMIZED_CALCULATION
			//
			#ifdef EXPANDED_CALCULATION
				//Build a vector from screen center to the current pixel
				//
				//Calculate distance across g.screen.
				//Subtract 1 because we are starting in the middle
				//of the first pixel, and ending in the middle of the
				//the last pixel. Our distance across the screen is
				//one less than the distance across all pixels.
				//This seems to work well.
				Flt ty = g.screen.fyres - 1.0;
				Flt tx = g.screen.fxres - 1.0;
				//Proportion of screen width & height
				//This is half the screen width & height
				Flt xprop = (Flt)px/tx;
				Flt yprop = (Flt)py/ty;
				//Position within vector
				//Multiply by 2 and subtract 1,
				//so that we move from negative to positive.
				Flt xpos = 2.0 * xprop - 1.0;
				Flt ypos = 2.0 * yprop - 1.0;
				//Position in frustum
				Flt xfrust =  frustumwidth  * xpos;
				Flt yfrust = -frustumheight * ypos;
				//Multiply by left and up vectors
				Vec h, v;
				h[0] = xfrust * left[0];
				h[1] = xfrust * left[1];
				h[2] = xfrust * left[2];
				v[0] = yfrust * up[0];
				v[1] = yfrust * up[1];
				v[2] = yfrust * up[2];
				//Add the vectors together to get a direction vector.
				//This is the direction from the center of the screen
				//to the pixel that we want to color.
				VecAdd(v, h, dir);
				//The magnitude of the dir vector will lead us directly
				//to the correct pixel.
				//
			#endif //EXPANDED_CALCULATION
			//
			#ifdef COMPRESSED_CALCULATION
				//
				VecComb(-frustumheight * (2.0 *
				        (Flt)py/(g.screen.fyres-1.0) - 1.0), up,
				        frustumwidth  * (2.0 *
				        (Flt)px/(g.screen.fxres-1.0) - 1.0), left,
				        dir);
				//
			#endif //COMPRESSED_CALCULATION
			//
			#ifdef OPTIMIZED_CALCULATION
				//
				//Define these temp variables above (outside the loops)
				//Flt ty = 1.0 / (g.screen.fyres - 1.0);
				//Flt tx = 1.0 / (g.screen.fxres - 1.0);
				VecComb(-frustumheight * (2.0 * (Flt)py*ty - 1.0), up,
				         frustumwidth  * (2.0 * (Flt)px*tx - 1.0), left,
				         dir);
				//
			#endif //OPTIMIZED_CALCULATION
			//
			//printf("pxpy: %i %i dir: %lf %lf %lf\n",px,py,dir[0],dir[1],dir[2]);
			VecCopy(dir, savedir);
			//
			if (g.cFlag) {
				VecAdd(dir, out, ray.d);
				//printf("from: %lf %lf %lf\n",ray.o[0],ray.o[1],ray.o[2]);
				//printf("  at: %lf %lf %lf\n",ray.o[0]+ray.d[0]*100.0,
				//                             ray.o[1]+ray.d[1]*100.0,
				//                             ray.o[2]+ray.d[2]*100.0);
				g.cFlag=0;
				extern void saveTempFile(Ray *ray, Flt angle);
				saveTempFile(&ray, g.studio.angle);
				g.tracing=0;
				return;
			}
			//
			//Now we trace the ray through the scene!
			//
			//Switch is used here because there are many different ways to
			//shoot the ray into the scene from the camera.
			//#define SHOOT_STRAIGHT     0x00000000
			//#define SHOOT_DISTRIBUTE4  0x00000010
			//#define SHOOT_APERTURE     0x00000100
			//#define SHOOT_MONTECARLO   0x00001000
			//#define SHOOT_ADAPTIVE     0x00010000
			#define SHOOT_1              0x00001100
			#define SHOOT_2              0x00011000
			//printf("%X\n",g.studio.shoot);
			switch (g.studio.shoot) {
				case SHOOT_STRAIGHT:
					//one ray.
					if (g.studio.jitter)
						jitter(dir);
					//Add vectors to get a vector from eye through a pixel
					VecAdd(dir, out, ray.d);
					VecNormalize(ray.d);
					trace(&ray, rgb, 1.0, 1, 0);
					break;
				case SHOOT_DISTRIBUTE4:
					cast_distribution(&ray, out, savedir, rgb, frustumstep);
					break;
				case SHOOT_APERTURE:
					cast_aperture_rays(&ray, out, savedir, rgb);
					break;
				case SHOOT_MONTECARLO:
					cast_montecarlo_rays(&ray, out, savedir, rgb,
					                     frustumstep, dim);
					break;
				case SHOOT_2:
					cast_montecarloA_rays(&ray, out, savedir, rgb,
					                      frustumstep, dim);
					break;
				case SHOOT_1:
					cast_aa_aperture_rays(&ray, out, savedir, rgb,
					                      frustumstep);
					break;
			}
			//Use the clamp function if colors get too large (white)
			//ClampRGB(rgb);
			//set block of pixels (might be 1-pixel)
			if (dim==1) {
				setpixel(j, i, rgb);
			} else {
				//The scene is pixelated with granularity > 1
				for (k=0; k<dim; k++) {
					if (i+k >= g.screen.yres)
						continue;
					for (m=0; m<dim; m++) {
						if (j+m >= g.screen.xres)
							continue;
						setpixel(j+m, i+k, rgb);
					}
				}
			}
		}
		//render each raster line as we go.
		//interval of 4 seems to be optimal.
		renderViewport(i, g.screen.xres, dim);
		if (++interval == g.renderInterval) {
			glXSwapBuffers(dpy, win);
			interval=0;
		}
		//
		//If user cancelled, then break out
		{
			XEvent e;
			while (XPending(dpy)) {
				XNextEvent(dpy, &e);
				checkKeys(&e);
			}
		}
		if (!g.tracing) break;
	}
	if (interval)
		glXSwapBuffers(dpy, win);
	if (g.aaOn) {
		g.studio.nsamples = save1;
		g.studio.var = save2;
		g.studio.shoot = save3;
	}
	g.tracing=0;
	g.readyToPrint=1;
	clock_gettime(CLOCK_REALTIME, &timerend);
	g.traceTime = timediff(&timerstart, &timerend);
	setTitle(g.traceTime);
}



