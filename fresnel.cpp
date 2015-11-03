//cs371 Introduction To Computer Graphics
//program:      fresnel.cpp
//author:       Gordon Griesel
//date written: Summer 2013
//purpose:      a framework for ray tracing
//
//This file has the makings of the famous Fresnel equations.
//
//In the ray tracer, we are probably using Sclick's approximation
//for reflectance and transmission of light.
//
//This file contains information to produce values based on
//the fresnel equations.
//
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "defs.h"
#include "log.h"
#include "extern.h"


Flt fresnelReflectance(Vec H, Vec V, Flt F0) {
	//---------------------------------------------------------------------------
	//14.3.1 Implementing a Physically Based Specular Reflectance Model for Skin
	//http://http.developer.nvidia.com/GPUGems3/gpugems3_ch14.html
	//Example 14-1. A Function for Computing the Fresnel Reflectance
	//in Specular BRDFs.
	//H is the standard half-angle vector.
	//F0 is reflectance at normal incidence (for skin use 0.028).
	//float fresnelReflectance( float3 H, float3 V, float F0 )
	//float base = 1.0 - dot( V, H );  
	//float exponential = pow( base, 5.0 );  
	//return exponential + F0 * ( 1.0 - exponential );  
	//H is halfway vector.
	//for specular reflection, H = surface normal
	//for specular highlight, H = halfway between view and light
	Flt cosTheta = VecDot(V,H);
	Flt exponential = pow(cosTheta, 5.0);
	return F0 + (1.0-F0) * (1.0-exponential);  
}

//---------------------------------------------------------------------------
//fresnel equation
// ni = ior outside
// nt = ior inside
// cos(Oi) VecDot(-ray.dir, nhit)
// cos(Ot) VecDot(refr.dir, -nhit)
// r_ = E0r / E0i = [ni * cos(Oi) - nt * cos(Ot)] / [ni * cos(Oi) + nt * cos(Ot)]
// t_ = E0t / E0i = 2ni * cos(Oi) / [ni * cos(Oi) + nt * cos(Ot)]
//---------------------------------------------------------------------------
void fresnelEquation(Flt ni, Flt nt, Ray *ray, Vec refr, Vec nhit, Flt *r, Flt *t) {
	Vec v;
	VecCopyNeg(ray->d, v);
	Flt cosOi = VecDot(v, nhit);
	VecCopyNeg(nhit, v);
	Flt cosOt = VecDot(refr, v);
	Flt E0r = ni * cosOi - nt * cosOt;
	Flt E0i = ni * cosOi + nt * cosOt;
	*r = E0r / E0i;
	Flt E0t = 2.0*ni * cosOi;
	*t = E0t / E0i;
}


