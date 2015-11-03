//cs371 Fall 2013
//program: sphere.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: framework for ray tracing
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "defs.h"
#include "log.h"
#include "vector.h"
#include "extern.h"

//notes:
//In the formulas, a 2 proceeding a variable is a multiply,
//                 a 2 following a variable is a squared.
//


int clipped(Vec p, Clip *ci)
{
	//This determines if a point is clipped or not.
	//Return:
	//        0 not clipped (visible)
	//        1 clipped     (not visible)
	Vec v;
	//Read through the linked list...
	Clip *c = ci;
	while (c) {
		if (c->radius > 0.0) {
			VecSub(p, c->center, v);
			Flt dist = VecLen(v);
			if (dist <= (c->radius*c->radius))
				return 1;
		} else {
			VecSub(p, c->center, v);
			if ((VecDot(v, c->norm)) > 0.0) {
				//point is on the "clipped" side of the plane
				return 1;
			}
		}
		c = c->next;
	}
	//point is not clipped
	return 0;
}


int clipped_cylinder(Vec p, Clip *ci)
{
	//This determines if a point is clipped or not.
	//Return:
	//        0 not clipped (visible)
	//        1 clipped     (not visible)
	Vec v;
	//Read through the linked list...
	Clip *c = ci;
	while(c) {
		//use un-transformed clip elements
		VecSub(p, c->ucenter, v);
		if ((VecDot(v, c->unorm)) > 0.0) {
			//point is on the "clipped" side of the plane
			return 1;
		}
		c = c->next;
	}
	//point is not clipped
	return 0;
}


int raySphereIntersect(Object *obj, Ray *ray, Hit *hit)
{
	//Determine if and where a ray intersects a sphere.
	//sp holds the sphere information
	Sphere *sp = (Sphere *)obj->obj;
	//Log("ray_sphere_intersect()...\n");
	//
	// sphere equation:
	// (p - c) * (p - c) = r * r
	//
	// where:
	// p = point on sphere surface
	// c = center of sphere
	//
	// ray equation:
	// o + t*d
	//
	// where:
	//   o = ray origin
	//   d = ray direction
	//   t = distance along ray, or scalar
	//
	// substitute ray equation into sphere equation
	//
	// (o + t*d - c) * (o + t*d - c) - r * r = 0
	//
	// we want it in this form:
	// a*t*t + b*t + c = 0
	//
	// (o + d*t - c)
	// (o + d*t - c)
	// -------------
	// o*o + o*d*t - o*c + o*d*t + d*t*d*t - d*t*c - o*c + c*d*t + c*c
	// d*t*d*t + o*o + o*d*t - o*c + o*d*t - d*t*c - o*c + c*d*t + c*c
	// d*t*d*t + 2(o*d*t) - 2(c*d*t) + o*o - o*c - o*c + c*c
	// d*t*d*t + 2(o-c)*d*t + o*o - o*c - o*c + c*c
	// d*t*d*t + 2(o-c)*d*t + (o-c)*(o-c)
	//
	// t*t*d*d + t*2*(o-c)*d + (o-c)*(o-c) - r*r
	//
	// a = dx*dx + dy*dy + dz*dz
	// b = 2(ox-cx)*dx + 2(oy-cy)*dy + 2(oz-cz)*dz
	// c = (ox-cx)*(ox-cx) + (oy-cy)*(oy-cy) + (oz-cz)*(oz-cz)
	//
	// now put it in quadratic form:
	// t = (-b +/- sqrt(b*b - 4ac)) / 2a
	//
	//Flt a = ray->d[0]*ray->d[0] + ray->d[1]*ray->d[1] + ray->d[2]*ray->d[2];
	//Flt b = 2.0*(ray->o[0]-sp->center[0])*ray->d[0] +
	//		2.0*(ray->o[1]-sp->center[1])*ray->d[1] +
	//		2.0*(ray->o[2]-sp->center[2])*ray->d[2];
	//Flt c = (ray->o[0]-sp->center[0])*(ray->o[0]-sp->center[0]) +
	//		(ray->o[1]-sp->center[1])*(ray->o[1]-sp->center[1]) +
	//		(ray->o[2]-sp->center[2])*(ray->o[2]-sp->center[2]) - sp->r2;
	//Flt t,t0,t1;
	////discriminant
	//Flt disc = b * b - 4.0 * a * c;
	//if (disc < 0.0) return 0;
	//disc = sqrt(disc);
	//t0 = (-b - disc) / (2.0*a);
	//t1 = (-b + disc) / (2.0*a);
	//
	//optimized from above...
	//Flt a = ray->d[0]*ray->d[0] + ray->d[1]*ray->d[1] + ray->d[2]*ray->d[2];
	//Flt b = (ray->o[0]-sp->center[0])*ray->d[0] +
	//		(ray->o[1]-sp->center[1])*ray->d[1] +
	//		(ray->o[2]-sp->center[2])*ray->d[2];
	//Flt c = (ray->o[0]-sp->center[0])*(ray->o[0]-sp->center[0]) +
	//		(ray->o[1]-sp->center[1])*(ray->o[1]-sp->center[1]) +
	//		(ray->o[2]-sp->center[2])*(ray->o[2]-sp->center[2]) - sp->r2;
	//Flt t,t0,t1;
	//discriminant
	//Flt disc = b * b - a * c;
	//if (disc < 0.0) return 0;
	//disc = sqrt(disc);
	//t0 = (-b - disc) / a;
	//t1 = (-b + disc) / a;
	//
	//optimized more from above...
	Vec v;
	VecSub(ray->o, sp->center, v);
	Flt a = VecDot(ray->d, ray->d);
	Flt b = VecDot(v, ray->d);
	Flt c = v[0] * (ray->o[0] - sp->center[0]) +
			v[1] * (ray->o[1] - sp->center[1]) +
			v[2] * (ray->o[2] - sp->center[2]) - sp->r2;
	Flt t,t0,t1;
	//discriminant
	Flt disc = b * b - a * c;
	if (disc < 0.0) {
		//Log("disc: %lf  returning no hit\n",disc);
		return 0;
	}
	disc = sqrt(disc);
	t0 = (-b - disc) / a;
	t1 = (-b + disc) / a;
	//
	if (t0 > 0.0) {
		//possible hit
		t = t0;
		RayPoint(ray,t,hit->p);
		if (sp->clip) {
			if (clipped(hit->p, sp->clip)) {
				goto checkspt1;
			}
		}
		hit->t = t;
		return 1;
	}
	checkspt1:
	if (t1 > 0.0) {
		t = t1;
		RayPoint(ray,t,hit->p);
		if (sp->clip) {
			if (clipped(hit->p, sp->clip)) {
				return 0;
			}
		}
		hit->t = t;
		return 1;
	}
	return 0;
}
//optimized sphere is here...
//	Flt b, disc, t0, t1, t, dot_vv;
//	Vec V;
//	VecSub(sp->center, ray->o, V);
//	b = VecDot(V, ray->d);
//	dot_vv = VecDot(V, V);
//	disc = b * b - dot_vv + sp->r2;
//	if (disc < 0.0)
//		return 0;
//	disc = sqrt(disc);
//	t0 = b - disc;
//	t1 = b + disc;


#define CYL_AT_ORIGIN

int rayCylinderIntersect(Object *obj, Ray *ray, Hit *hit)
{
	Cylinder *cy = (Cylinder *)obj->obj;
	//
	Ray rx;
	Surface *sf = obj->surface;
	transVector(sf->invmat, ray->o, rx.o);
	transNormal(sf->invmat, ray->d, rx.d);
	//
	//http://voices.yahoo.com/developing-equation-cone-simplest-case-2522846.html?cat=17
	// x2 + z2 = a*y2
	//-----------------------------
	//
	//----------------------------------
	//for cylinder centered at origin...
	//----------------------------------
	//
	// cylinder equation
	// x2 + z2 = r2
	//
	// where:
	//   x = x component of point on cylinder surface
	//   z = z component of point on cylinder surface
	//   r = radius
	//
	// ray equation:
	// o + t*d
	//
	// where:
	//   o = ray origin
	//   d = ray direction
	//   t = distance along ray, or scalar
	//
	// substitute ray equation into sphere equation
	//
	// (ox+t*dx)2 + (oz+t*dz)2 - r*r = 0
	//
	// where:
	//   ox = x component of ray origin
	//   oz = z component of ray origin
	//   dx = x component of ray direction
	//   dz = z component of ray direction
	//
	// ox + t*dx
	// ox + t*dx
	// --------------------------------
	// ox*ox + 2(ox * t*dx) + t*t*dx*dx
	//
	// add in the z components...
	//
	//ox*ox + 2(ox * t*dx) + t*t*dx*dx + oz*oz + 2(oz * t*dz) + t*t*dz*dz - r*r
	//
	//Goal is to solve for t using the quadratic equation...
	// t = (-b +/- sqrt(b*b - 4ac)) / 2a
	//
	//t*t*dx*dx + t*t*dz*dz + ox*ox + 2(ox * t*dx) + oz*oz + 2(oz * t*dz) - r*r
	//t*t*dx*dx + t*t*dz*dz + 2(t*ox*dx) + 2(t*oz*dz) + ox*ox + oz*oz - r*r
	//t*t*dx*dx + t*t*dz*dz + 2t*ox*dx + 2t*oz*dz + ox*ox + oz*oz - r*r
	//t*t*dx*dx + t*t*dz*dz + t*2*ox*dx + t*2*oz*dz + ox*ox + oz*oz - r*r
	//a = dx*dx + dz*dz
	//b = 2*ox*dx + 2*oz*dz
	//c = ox*ox + oz*oz - r*r
	//
	//now put a,b,c into C source code...
	Flt a = rx.d[0] * rx.d[0] + rx.d[2] * rx.d[2];
	Flt b = 2.0 * rx.o[0] * rx.d[0] + 2.0 * rx.o[2] * rx.d[2];
	Flt c = rx.o[0]*rx.o[0] + rx.o[2]*rx.o[2] - cy->radius*cy->radius;
	//
	Flt t0,t1;
	//disc:  discriminant
	Flt disc = b * b - 4.0 * a * c;
	if (disc < 0.0) return 0;
	disc = sqrt(disc);
	t0 = (-b - disc) / (2.0*a);
	t1 = (-b + disc) / (2.0*a);
	if (t0 > 0.0) {
		//possible hit
		//t = t0;
		RayPoint(&rx,t0,hit->p);
		if (hit->p[1] >= 0.0 && hit->p[1] <= cy->apex) {
			if (cy->clip) {
				if (clipped_cylinder(hit->p, cy->clip)) {
					goto checkcyt1;
				}
			}
			transVector(sf->mat, hit->p, hit->p);
			hit->t = t0;
			return 1;
		}
	}
	checkcyt1:
	if (t1 > 0.0) {
		//t = t1;
		RayPoint(&rx,t1,hit->p);
		if (hit->p[1] >= 0.0 && hit->p[1] <= cy->apex) {
			if (cy->clip) {
				if (clipped_cylinder(hit->p, cy->clip)) {
					return 0;
				}
			}
			transVector(sf->mat, hit->p, hit->p);
			hit->t = t1;
			return 1;
		}
	}
	return 0;
}


/*
int ray_cylinder_intersect(Object *obj, Ray *ray, Hit *hit)
{
	Cylinder *cy = obj->obj;
	//
	//http://voices.yahoo.com/developing-equation-cone-simplest-case-2522846.html?cat=17
	// x2 + z2 = a*y2
	//-----------------------------
	//
	#ifdef CYL_AT_ORIGIN
	//----------------------------------
	//for cylinder centered at origin...
	//----------------------------------
	//
	// cylinder equation
	// x2 + z2 = r2
	//
	// where:
	//   x = x component of point on cylinder surface
	//   z = z component of point on cylinder surface
	//   r = radius
	//
	// ray equation:
	// o + t*d
	//
	// where:
	//   o = ray origin
	//   d = ray direction
	//   t = distance along ray, or scalar
	//
	// substitute ray equation into sphere equation
	//
	// (ox+t*dx)2 + (oz+t*dz)2 - r*r = 0
	//
	// where:
	//   ox = x component of ray origin
	//   oz = z component of ray origin
	//   dx = x component of ray direction
	//   dz = z component of ray direction
	//
	// ox + t*dx
	// ox + t*dx
	// --------------------------------
	// ox*ox + 2(ox * t*dx) + t*t*dx*dx
	//
	// add in the z components...
	//
	//ox*ox + 2(ox * t*dx) + t*t*dx*dx + oz*oz + 2(oz * t*dz) + t*t*dz*dz - r*r
	//
	//Goal is to solve for t using the quadratic equation...
	// t = (-b +/- sqrt(b*b - 4ac)) / 2a
	//
	//t*t*dx*dx + t*t*dz*dz + ox*ox + 2(ox * t*dx) + oz*oz + 2(oz * t*dz) - r*r
	//t*t*dx*dx + t*t*dz*dz + 2(t*ox*dx) + 2(t*oz*dz) + ox*ox + oz*oz - r*r
	//t*t*dx*dx + t*t*dz*dz + 2t*ox*dx + 2t*oz*dz + ox*ox + oz*oz - r*r
	//t*t*dx*dx + t*t*dz*dz + t*2*ox*dx + t*2*oz*dz + ox*ox + oz*oz - r*r
	//a = dx*dx + dz*dz
	//b = 2*ox*dx + 2*oz*dz
	//c = ox*ox + oz*oz - r*r
	//
	//now put a,b,c into C source code...
	Flt a = ray->d[0] * ray->d[0] + ray->d[2] * ray->d[2];
	Flt b = 2.0 * ray->o[0] * ray->d[0] + 2.0 * ray->o[2] * ray->d[2];
	Flt c = ray->o[0]*ray->o[0] + ray->o[2]*ray->o[2] - cy->radius*cy->radius;
	#endif //CYL_AT_ORIGIN
	//
	#ifdef CYL_AT_ITS_CENTER
	//for cylinder centered at c...
	//
	// cylinder equation:
	//
	// (x-cx)2 + (z-cz)2 = r2
	//
	// where:
	//   x = x component of point on cylinder surface
	//   z = z component of point on cylinder surface
	//   cx = x component of cylinder center
	//   cz = z component of cylinder center
	//   r = radius
	//   2 following an expression means squared
	//
	// now substitute in the ray equation
	//
	//(ox+t*dx-cx)2 + (cz-oz+t*dz-cz)2 - r*r = 0
	//  ox + t*dx - cx
	//  ox + t*dx - cx
	//  --------------------------------------------------------------------------------
	//  ox*ox + ox*t*dx -ox*cx + ox*t*dx + t*dx*t*dx - t*dx*cx - ox*cx - t*dx*cx + cx*cx
	//
	//t*t*dx*dx + t*ox*dx + t*ox*dx - t*dx*cx - t*dx*cx + ox*ox - ox*cx - ox*cx + cx*cx
	//add the z components (similar to x)
	//a = dx*dx + dz*dz
	//b = 2*ox*dx - 2*dx*cx + 2*oz*dz - 2*dz*cz
	//c = ox*ox - 2*ox*cx + cx*cx + oz*oz - 2*oz*cz + cz*cz - r*r
	//
	Flt a = ray->d[0]*ray->d[0] + ray->d[2]*ray->d[2];
	Flt b = 2.0*ray->o[0]*ray->d[0] - 2.0*ray->d[0]*cy->center[0] +
			2.0*ray->o[2]*ray->d[2] - 2.0*ray->d[2]*cy->center[2];
	Flt c = ray->o[0]*ray->o[0] - 2.0*ray->o[0]*cy->center[0] +
			cy->center[0]*cy->center[0] + ray->o[2]*ray->o[2] -
			2.0*ray->o[2]*cy->center[2] + cy->center[2]*cy->center[2] - cy->radius*cy->radius;
	#endif //CYL_AT_ITS_CENTER

	Flt t,t0,t1;
	//discriminant
	Flt disc = b * b - 4.0 * a * c;
	if (disc < 0.0) return 0;
	disc = sqrt(disc);
	t0 = (-b - disc) / (2.0*a);
	t1 = (-b + disc) / (2.0*a);
	if (t0 > 0.0) {
		//possible hit
		t = t0;
		RayPoint(ray,t,hit->p);
		if (hit->p[1] >= cy->center[1] + cy->base &&
			hit->p[1] <= cy->center[1] + cy->apex) {
			if (cy->clip) {
				if (clipped(hit->p, cy->clip)) {
					t = t1;
					RayPoint(ray,t,hit->p);
					if (cy->clip) {
						if (clipped(hit->p, cy->clip)) {
							return 0;
						}
					}
				}
			}
			hit->t = t;
			return 1;
		}
	}
	if (t1 > 0.0) {
		t = t1;
		RayPoint(ray,t,hit->p);
		if (hit->p[1] >= cy->center[1] + cy->base &&
			hit->p[1] <= cy->center[1] + cy->apex) {
			if (cy->clip) {
				if (clipped(hit->p, cy->clip)) {
					return 0;
				}
			}
			hit->t = t;
			return 1;
		}
	}
	return 0;
}
*/



//#define CONE_AT_ORIGIN
#define CONE_AT_ITS_CENTER

int rayConeIntersect(Object *obj, Ray *ray, Hit *hit)
{
	Cone *cn = (Cone *)obj->obj;
	//return 0;

	#ifdef CONE_AT_ORIGIN
	//----------------------------------
	//for cone centered at origin...
	//----------------------------------
	//
	//general cone equation
	//
	//x2 + z2 = (baseRadius/height)2 * (apex-y)2
	//
	//where:
	//   x = x component of point on cone surface
	//   y = y component of point on cone surface
	//   z = z component of point on cone surface
	//   baseRadius = cone radius at base
	//   height = height of cone (apex - base)
	//
	//
	//plug in the ray equation
	//
	//(ox+t*dx)2 + (oz+t*dz)2 - (baseRadius/height)2 * (apex-y)2 = 0
	//
	//let k = baseRadius/height
	//
	//(ox+t*dx)2 + (oz+t*dz)2 - k*k*(apex - (oy - t*dy))2 = 0
	//(ox+t*dx)2 + (oz+t*dz)2 - k*k*((apex - oy) + t*dy)2 = 0
	//
	//let yk = (apex - oy)
	//k*k*(yk + t*dy)2
	//yk + t*dy
	//yk + t*dy
	//--------------------------------------------
	//yk*yk*k*k + t*2*yk*dy*k*k + t*t*dy*dy*k*k
	//
	//
	//ox + t*dx
	//ox + t*dx
	//--------------------------------------
	//ox*ox + 2(ox * t*dx) + t*t*dx*dx
	//
	//ox*ox + 2(ox * t*dx) + t*t*dx*dx + oz*oz + 2(oz * t*dz) + t*t*dz*dz - r*r
	//t*t*dx*dx + t*t*dz*dz + ox*ox + 2(ox * t*dx) + oz*oz + 2(oz * t*dz) - r*r
	//t*t*dx*dx + t*t*dz*dz + 2(t*ox*dx) + 2(t*oz*dz) + ox*ox + oz*oz - r*r
	//t*t*dx*dx + t*t*dz*dz + 2t*ox*dx + 2t*oz*dz + ox*ox + oz*oz - r*r
	//t*t*dx*dx + t*t*dz*dz + t*2*ox*dx + t*2*oz*dz + ox*ox + oz*oz - r*r
	//
	//t*t*dx*dx + t*t*dz*dz - t*t*dy*dy*k*k +
	//t*2*ox*dx + t*2*oz*dz - t*2*yk*dy*k*k +
	//ox*ox + oz*oz - yk*yk*k*k
	//
	//a = dx*dx + dz*dz - dy*dy*k*k
	//b = 2*ox*dx + 2*oz*dz - 2*yk*dy*k*k
	//c = ox*ox + oz*oz - yk*yk*k*k
	//
	Flt k = cn->radius/cn->apex;
	Flt yk = ray->o[1] - cn->apex;
	Flt a = ray->d[0] * ray->d[0] + ray->d[2] * ray->d[2] - ray->d[1]*ray->d[1]*k*k;
	Flt b = 2.0 * (ray->o[0] * ray->d[0] + ray->o[2] * ray->d[2] - yk*ray->d[1]*k*k);
	Flt c = ray->o[0]*ray->o[0] + ray->o[2]*ray->o[2] - yk*yk*k*k;
	Flt t,t0,t1;
	//discriminant
	Flt disc = b * b - 4.0 * a * c;
	if (disc < 0.0) return 0;
	disc = sqrt(disc);
	t0 = (-b - disc) / (2.0*a);
	t1 = (-b + disc) / (2.0*a);
	if (t0 > 0.0) {
		//possible hit
		t = t0;
		RayPoint(ray,t,hit->p);
		if (hit->p[1] >= cn->center[1] + cn->base &&
			hit->p[1] <= cn->center[1] + cn->apex) {
			if (cn->clip) {
				if (clipped(hit->p, cn->clip)) {
					goto checkcnt1;
			//		t = t1;
			//		RayPoint(ray,t,hit->p);
			//		if (cn->clip) {
			//			if (clipped(hit->p, cn->clip)) {
			//				return 0;
			//			}
			//		}
				}
			}
			hit->t = t;
			return 1;
		}
	}
	checkcnt1:
	if (t1 > 0.0) {
		t = t1;
		RayPoint(ray,t,hit->p);
		if (hit->p[1] >= cn->center[1] + cn->base &&
			hit->p[1] <= cn->center[1] + cn->apex) {
			if (cn->clip) {
				if (clipped(hit->p, cn->clip)) {
					return 0;
				}
			}
			hit->t = t;
			return 1;
		}
	}
	return 0;
	#endif //CONE_AT_ORIGIN

	#ifdef CONE_AT_ITS_CENTER
	//----------------------------------
	//for cone centered at c...
	//----------------------------------
	//We are going to translate the ray, not the cone.
	Ray tray;
	VecCopy(ray->d, tray.d);
	//Offset a temporary ray by the inverse of the cone center
	tray.o[0] = ray->o[0] - cn->center[0];
	tray.o[1] = ray->o[1] - cn->center[1];
	tray.o[2] = ray->o[2] - cn->center[2];
	//
	Flt k = cn->radius/cn->apex;
	Flt yk = tray.o[1] - cn->apex;
	Flt k2 = k*k;
	Flt a = tray.d[0] * tray.d[0] + tray.d[2] * tray.d[2] - tray.d[1]*tray.d[1]*k2;
	Flt b = 2.0 * (tray.o[0] * tray.d[0] + tray.o[2] * tray.d[2] - yk*tray.d[1]*k2);
	Flt c = tray.o[0]*tray.o[0] + tray.o[2]*tray.o[2] - yk*yk*k2;
	Flt t,t0,t1;
	//discriminant
	Flt disc = b * b - 4.0 * a * c;
	if (disc < 0.0) return 0;
	disc = sqrt(disc);
	t0 = (-b - disc) / (2.0*a);
	t1 = (-b + disc) / (2.0*a);
	if (t0 > 0.0) {
		//possible hit
		t = t0;
		RayPoint(&tray,t,hit->p);
		VecAdd(hit->p, cn->center, hit->p);
		if (hit->p[1] >= cn->center[1] + cn->base &&
			hit->p[1] <= cn->center[1] + cn->apex) {
			if (cn->clip) {
				if (clipped(hit->p, cn->clip)) {
					t = t1;
					RayPoint(&tray,t,hit->p);
					VecAdd(hit->p, cn->center, hit->p);
					if (cn->clip) {
						if (clipped(hit->p, cn->clip)) {
							return 0;
						}
					}
				}
			}
			hit->t = t;
			return 1;
		}
	}
	if (t1 > 0.0) {
		t = t1;
		RayPoint(&tray,t,hit->p);
		VecAdd(hit->p, cn->center, hit->p);
		if (hit->p[1] >= cn->center[1] + cn->base &&
			hit->p[1] <= cn->center[1] + cn->apex) {
			if (cn->clip) {
				if (clipped(hit->p, cn->clip)) {
					return 0;
				}
			}
			hit->t = t;
			return 1;
		}
	}
	return 0;
	#endif //CONE_AT_ITS_CENTER
}

int rayPlaneIntersect(Vec ray_o, Vec ray_d,
							Vec plane_center, Vec plane_norm, Hit *hit)
{
	//Two perpendicular vectors have a dot-product of 0
	//A plane can be defined as a point p0 and a normal n
	//Any point p on the plane can be expressed:
	//
	//(p - p0) . n = 0
	//
	//where:
	//  p  = any point on the plane
	//  p0 = a known point on the plane
	//  n  = unit normal of plane
	//  .  = dot product
	//
	//Only points on the plane will have this relationship.
	//
	//Our ray equation:
	//
	//o + t*d
	//
	//where:
	//  o = ray origin
	//  d = ray direction
	//  t = distance along ray, or scalar
	//
	//Substitute ray equation for p
	//
	//(o + t*d - p0) . n = 0
	//(t*d + o - p0) . n = 0
	//
	//note: dot product is distributive
	//
	//t*d . n + (o - p0) . n = 0
	//t*d . n = -(o - p0) . n
	//t = -(o - p0) . n / d . n
	//t = (p0 - o) . n / d . n
	//
	Vec v;
	Flt dot1, dot2, t;
	//
	//note: a return of 0 means no hit
	dot1 = VecDot(ray_d, plane_norm);
	if (dot1 == 0.0)
		return 0;
	VecSub(plane_center, ray_o, v);
	dot2 = VecDot(plane_norm, v);
	t = dot2 / dot1;
	if (t < 0.0)
		return 0;
	//Hit point is along the ray
	//Calculate the hit point, and return 1
	hit->t = t;
	hit->p[0] = ray_o[0] + (ray_d[0] * t);
	hit->p[1] = ray_o[1] + (ray_d[1] * t);
	hit->p[2] = ray_o[2] + (ray_d[2] * t);
	return 1;
}

int rayRingIntersect(Object *obj, Ray *ray, Hit *hit)
{
	Ring *r = (Ring *)obj->obj;
	Flt dist;
	Vec v;
	if (rayPlaneIntersect(ray->o, ray->d, r->center, r->norm, hit)) {
		//distance from disc center to hit point?
		VecSub(hit->p, r->center, v);
		dist = VecLenSq(v);
		if (dist <= r->maxradius && dist >= r->minradius) {
			if (r->clip) {
				if (clipped(hit->p, r->clip))
					return 0;
			}
			return 1;
		}
	}
	return 0;
}

int Barycentric(Vec a, Vec b, Vec c, Vec p, Flt *u, Flt *v)
{
	//From Christer Ericson's Real-Time Collision Detection
	Vec v0,v1,v2;
	VecSub(b, a, v0);
	VecSub(c, a, v1);
	VecSub(p, a, v2);
	Flt d00 = VecDot(v0, v0);
	Flt d01 = VecDot(v0, v1);
	Flt d11 = VecDot(v1, v1);
	Flt d20 = VecDot(v2, v0);
	Flt d21 = VecDot(v2, v1);
	Flt denom = d00 * d11 - d01 * d01;
	if (denom == 0.0) return 0;
	*u = (d11 * d20 - d01 * d21) / denom;
	*v = (d00 * d21 - d01 * d20) / denom;
	return (*u >= 0.0 && *v >= 0.0 && *u + *v <= 1.0);
}

int rayTriIntersect(Object *obj, Ray *r, Hit *h)
{
	Tri *t = (Tri *)obj->obj;
	if (rayPlaneIntersect(r->o, r->d, t->verts[0], t->norm, h)) {
		Flt u,v;
		if (Barycentric(t->verts[0], t->verts[1], t->verts[2],
													h->p, &u, &v)) {
			t->uvw[0] = u;
			t->uvw[1] = v;
			t->uvw[2] = 1.0 - v - u;
			//assert...
			//if (t->uvw[0] > 1.0 ||
			//	t->uvw[1] > 1.0 ||
			//	t->uvw[2] > 1.0) {
			//	exit(0);
			//}
			if (t->clip) {
				if (clipped(h->p, t->clip))
					return 0;
			}
			return 1;
		}
	}
	return 0;
}

void sphereNormal(Object *obj, Hit *hit, Vec norm)
{
	Sphere *s = (Sphere *)obj->obj;
	VecSub(hit->p, s->center, norm);
}

void ringNormal(Object *obj, Hit *hit, Vec norm)
{
	Ring *r = (Ring *)obj->obj;
	VecCopy(r->norm, norm);
	//so compiler won't complain
	if (hit->t){}
}

void triNormal(Object *obj, Hit *hit, Vec norm)
{
	Tri *t = (Tri *)obj->obj;
	VecCopy(t->norm, norm);
	#ifdef ALLOW_PATCHES
	if (t->patch) {
		int u=2,v=0,w=1;
		norm[0] =	t->pnorm[0][0] * t->uvw[u]+
					t->pnorm[1][0] * t->uvw[v]+
					t->pnorm[2][0] * t->uvw[w];
		norm[1] =	t->pnorm[0][1] * t->uvw[u]+
					t->pnorm[1][1] * t->uvw[v]+
					t->pnorm[2][1] * t->uvw[w];
		norm[2] =	t->pnorm[0][2] * t->uvw[u]+
					t->pnorm[1][2] * t->uvw[v]+
					t->pnorm[2][2] * t->uvw[w];
	}
	#endif //ALLOW_PATCHES
	//so compiler won't complain
	if (hit->t){}
}

void cylinderNormal(Object *obj, Hit *hit, Vec norm)
{
	//Un-transform the hit point
	Vec p;
	Surface *sf = obj->surface;
	transVector(sf->invmat, hit->p, p);
	//
	//Get the normal
	//Center of cylinder is at the origin, remember
	MakeVector(p[0], 0, p[2], norm);
	//
	//Now, transform it into our scene space
	transNormal(sf->mat, norm, norm);
}

void coneNormal(Object *obj, Hit *hit, Vec norm)
{
	Cone *cn = (Cone *)obj->obj;
	//http://www.ctralie.com/PrincetonUGRAD/Projects/COS426/Assignment3/part1.html#raycone
	//To calculate the normal, first subtract the cone's center from the ray's endpoint,
	//and project it along the XZ plane. To determine the y-component of the normal,
	//multiply the magnitude of the xz part of the normal by tan(A)
	//(as shown in the diagram on link), where tan(A) is (r/h).
	norm[0] = hit->p[0] - cn->center[0];
	norm[1]=0.0;
	norm[2] = hit->p[2] - cn->center[2];
	//Flt magnitude = VecLenSq(norm);
	//norm[1] = magnitude * (cn->radius/cn->apex);
	norm[1] = (VecLenSq(norm)) * (cn->radius/cn->apex);
}


