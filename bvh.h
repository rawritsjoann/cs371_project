#ifndef _BVH_H_
#define _BVH_H_

//cs371 Introduction To Computer Graphics
//program:      bvh.h
//author:       Gordon Griesel
//date written: Summer 2013
//purpose:      a framework for ray tracing
//
//
//Preprocessor flags used in building and using the BVH
//Bounding volume hierarchy


//The following defines allow performance testing
//Comment to turn off
//BVH is Bounding volume hierarchy

#define USE_BVH_FOR_TRACING
#define USE_BVH_FOR_SHADOWS
//#define USE_SHADOW_CACHING


//This will check for the best axis to split on
//
//0 = no checking
//1 = largest width
//2 = most objects after axis divide
//3 = random axis determination
//
#define BVH_AXIS_CHECK 1

//How many objects must be in a box to divide it?

#define MIN_BUNCHING_OBJECTS 11


#endif

