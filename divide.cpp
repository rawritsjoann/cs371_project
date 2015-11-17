//cs371 Fall 2013
//program: divide.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: framework for ray tracing
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "defs.h"
#include "log.h"
#include "extern.h"

//for Bounding volume hierarchy
#include "bvh.h"

int checkSlabs(Bbox *b, Ray *r)
{
	//Axis alligned bounding box
	//Box is formed by the intersection of slabs
	//source for this code:
	//http://tavianator.com/2011/05/fast-branchless-raybounding-box-intersections/
	Flt tmin = -9e22, tmax = 9e22;
	//Does ray start inside bounding volume?
	Vec v = { r->o[0], r->o[1], r->o[2] };
	if (v[0] >= b->min[0] && v[0] <= b->max[0] &&
		v[1] >= b->min[1] && v[1] <= b->max[1] &&
		v[2] >= b->min[2] && v[2] <= b->max[2]) {
		//ray starts within the bounding box.
		return 1;
	}
	if (r->d[0] != 0.0) {
		Flt div = 1.0 / r->d[0];
		Flt tx1 = (b->min[0] - v[0]) * div;
		Flt tx2 = (b->max[0] - v[0]) * div;
		tmin = fmax(tmin, fmin(tx1, tx2));
		tmax = fmin(tmax, fmax(tx1, tx2));
	}
	if (r->d[1] != 0.0) {
		Flt div = 1.0 / r->d[1];
		Flt tx1 = (b->min[1] - v[1]) * div;
		Flt tx2 = (b->max[1] - v[1]) * div;
		tmin = fmax(tmin, fmin(tx1, tx2));
		tmax = fmin(tmax, fmax(tx1, tx2));
	}
	if (r->d[2] != 0.0) {
		Flt div = 1.0 / r->d[2];
		Flt tx1 = (b->min[2] - v[2]) * div;
		Flt tx2 = (b->max[2] - v[2]) * div;
		tmin = fmax(tmin, fmin(tx1, tx2));
		tmax = fmin(tmax, fmax(tx1, tx2));
	}
	return (tmax >= tmin);
}

#define MIN(a,b) (a) <= (b) ? (a) : (b)

void divide(BVH *bvh)
{
	//Log("divide()...\n");
	//This function is recursive
	int nobjects, largesub, done;
	Object *o, *newo, *saveo, *newListHead;
	//
	if (!bvh) return;
	//
	//Create two new bounding volumes
	Bbox b1,b2;
	memcpy(&b1, &bvh->bb, sizeof(Bbox));
	memcpy(&b2, &bvh->bb, sizeof(Bbox));
	//Split along an axis of X or Y or Z

	if (BVH_AXIS_CHECK==0) {
		//No decision about splitting the box
		b1.max[0] = (b1.min[0] + b1.max[0]) / 2.0;
		b2.min[0] = b1.max[0];
	}
	if (BVH_AXIS_CHECK==1) {
		//Largest span
		Vec axis;
		VecSub(b1.max, b1.min, axis);
		//
		largesub=0;
		if (axis[1] >= axis[0] && axis[1] >= axis[2]) largesub=1;
		if (axis[2] >= axis[0] && axis[2] >= axis[1]) largesub=2;
		//
		b1.max[largesub] = (b1.min[largesub] + b1.max[largesub]) / 2.0;
		b2.min[largesub] = b1.max[largesub];
	}
	if (BVH_AXIS_CHECK==2) {
		//Most objects on both sides
		int check_axis_balance(int axis, Bbox *bb, BVH *bvh);
		int c0 = check_axis_balance(0, &b1, bvh);
		int c1 = check_axis_balance(1, &b1, bvh);
		int c2 = check_axis_balance(2, &b1, bvh);
		//Log("c0: %i c1: %i c2: %i\n",c0,c1,c2);
		//Which count is largest?
		int ax = -1;
		if (c0 > 0 && c0 >= c1 && c0 >= c2) ax = 0;
		if (c1 > 0 && c1 >= c0 && c1 >= c2) ax = 1;
		if (c2 > 0 && c2 >= c0 && c2 >= c1) ax = 2;
		if (ax < 0) return;
		//Log("ax: %i\n", ax);
		b1.max[ax] = (b1.min[ax] + b1.max[ax]) / 2.0;
		b2.min[ax] = b1.max[ax];
		//Log("%lf %lf %lf\n",b1.max[0]-b1.min[0],b1.max[1]-b1.min[1],b1.max[2]-b1.min[2]);
	}
	if (BVH_AXIS_CHECK==3) {
		//Random decision about splitting the box
		int ax = random(2);
		b1.max[ax] = (b1.min[ax] + b1.max[ax]) / 2.0;
		b2.min[ax] = b1.max[ax];
	}
	//
	//Look for objects completely inside left box
	done=0;
	nobjects=0;
	newListHead=NULL;
	o = bvh->ohead;
	while(o) {
		if (o->bb.min[0] >= b1.min[0] &&
			o->bb.min[1] >= b1.min[1] &&
			o->bb.min[2] >= b1.min[2] &&
			o->bb.max[0] <= b1.max[0] &&
			o->bb.max[1] <= b1.max[1] &&
			o->bb.max[2] <= b1.max[2]
		) {
			//completely inside
			//add to new list, delete from existing list
			saveo = o->next;
			if (o->prev == NULL) {
				if (o->next == NULL) {
					//only one item in list
					bvh->ohead=NULL;
					done=1;
				} else {
					//at beginning of list
					o->next->prev = NULL;
					bvh->ohead = o->next;
				}
			} else {
				if (o->next == NULL) {
					//at end of list
					o->prev->next = NULL;
				} else {
					//in middle of list
					o->prev->next = o->next;
					o->next->prev = o->prev;
				}
			}
			newo = o;
			//insert new object into linked-list.
			if (newListHead) newListHead->prev = newo;
			newo->next = newListHead;
			newListHead = newo;
			newListHead->prev = NULL;
			nobjects++;
			if (done) {
				//the list ran out when we removed o above.
				break;
			}
			o = saveo;
			continue;
		}
		o = o->next;
	}
	//
	//Log("l nobjects: %i\n",nobjects);
	if (nobjects > 0) {
		BVH *bvh1 = (BVH *)malloc(sizeof(BVH));
		bvh1->left = NULL;
		bvh1->right = NULL;
		bvh1->ohead = newListHead;
		memcpy(&bvh1->bb, &b1, sizeof(Bbox));
		bvh->left = bvh1;
		//If enough objects inside, then divide again
		if (nobjects > MIN_BUNCHING_OBJECTS) {
			divide(bvh->left);
		}
	}
	//
	//
	//
	//Look for objects completely inside right box
	done=0;
	nobjects=0;
	newListHead=NULL;
	o = bvh->ohead;
	while(o) {
		if (o->bb.min[0] >= b2.min[0] &&
			o->bb.min[1] >= b2.min[1] &&
			o->bb.min[2] >= b2.min[2] &&
			o->bb.max[0] <= b2.max[0] &&
			o->bb.max[1] <= b2.max[1] &&
			o->bb.max[2] <= b2.max[2]
		) {
			//completely inside
			//add to new list, delete from existing list
			saveo = o->next;
			if (o->prev == NULL) {
				if (o->next == NULL) {
					//only one item in list
					bvh->ohead=NULL;
					done=1;
				} else {
					//at beginning of list
					o->next->prev = NULL;
					bvh->ohead = o->next;
				}
			} else {
				if (o->next == NULL) {
					//at end of list
					o->prev->next = NULL;
				} else {
					//in middle of list
					o->prev->next = o->next;
					o->next->prev = o->prev;
				}
			}
			newo = o;
			//insert new object into linked-list.
			if (newListHead) newListHead->prev = newo;
			newo->next = newListHead;
			newListHead = newo;
			newListHead->prev = NULL;
			nobjects++;
			if (done) {
				//the list ran out when we removed o above.
				break;
			}
			o = saveo;
			continue;
		}
		o = o->next;
	}
	//
	//Log("r nobjects: %i\n",nobjects);
	if (nobjects > 0) {
		BVH *bvh2 = (BVH *)malloc(sizeof(BVH));
		bvh2->left = NULL;
		bvh2->right = NULL;
		bvh2->ohead = newListHead;
		memcpy(&bvh2->bb, &b2, sizeof(Bbox));
		bvh->right = bvh2;
		//If enough objects inside, then divide again
		if (nobjects > MIN_BUNCHING_OBJECTS) {
			divide(bvh->right);
		}
	}
}

void buildHierarchy(void)
{
	int j;
	//Log("build_hierarchy()...\n");
	//Initial bounding box object
	g.bvhHead = (BVH *)malloc(sizeof(BVH));
	g.bvhHead->left = NULL;
	g.bvhHead->right = NULL;
	//Put all objects into list of main BB
	g.bvhHead->ohead = NULL;
	MakeVector( 9e9, 9e9, 9e9,g.bvhHead->bb.min);
	MakeVector(-9e9,-9e9,-9e9,g.bvhHead->bb.max);
	//Process all objects in the scene, and create the
	//top-level bounding volume.
	Object *o = g.objectHead;
	while(o) {
		Object *newob = (Object *)malloc(sizeof(Object));
		memcpy(newob, o, sizeof(Object));
		//Update the bounding volume dimensions...
		for (j=0; j<3; j++) {
			if (g.bvhHead->bb.min[j] > o->bb.min[j])
				g.bvhHead->bb.min[j] = o->bb.min[j];
			if (g.bvhHead->bb.max[j] < o->bb.max[j])
				g.bvhHead->bb.max[j] = o->bb.max[j];
		}
		//insert new object into linked-list.
		if (g.bvhHead->ohead) g.bvhHead->ohead->prev = newob;
		newob->next = g.bvhHead->ohead;
		g.bvhHead->ohead = newob;
		g.bvhHead->ohead->prev = NULL;
		//move to next object
		o = o->next;
	}
	divide(g.bvhHead);
	//
	//
	//let's look at the structure of the BVH
	//void show_bvh_structure(BVH *bvh);
	//show_bvh_structure(bvhHead);
}


void show_bvh_structure(BVH *bvh)
{
	if (!bvh) return;
	int nobjects=0;
	Object *o;
	BVH *b = bvh;
	o = b->ohead;
	while(o) {
		if (o->type==OBJECT_TYPE_SPHERE) {
			Sphere *s = (Sphere *)o->obj;
			Log("sp: %lf %lf %lf   %lf\n",s->center[0],s->center[1],s->center[2],s->radius);
		}
		nobjects++;
		o = o->next;
	}
	Log("bvh: %i  %lf %lf %lf  %lf %lf %lf\n",
			nobjects,
			b->bb.min[0],
			b->bb.min[1],
			b->bb.min[2],
			b->bb.max[0],
			b->bb.max[1],
			b->bb.max[2]);
	show_bvh_structure(b->left);
	show_bvh_structure(b->right);
}


int check_axis_balance(int axis, Bbox *bb, BVH *bvh)
{
	int nobjects[2]={0,0};
	Object *o;
	Bbox b1,b2;
	memcpy(&b1, bb, sizeof(Bbox));
	memcpy(&b2, bb, sizeof(Bbox));
	//Split down the middle
	b1.max[axis] = (b1.min[axis] + b1.max[axis]) / 2.0;
	b2.min[axis] = b1.max[axis];
	//
	//We have our 2 boxes
	//How many objects completely inside each?
	//
	//Look for objects completely inside left box
	o = bvh->ohead;
	while(o) {
		if (o->bb.min[0] >= b1.min[0] &&
			o->bb.min[1] >= b1.min[1] &&
			o->bb.min[2] >= b1.min[2] &&
			o->bb.max[0] <= b1.max[0] &&
			o->bb.max[1] <= b1.max[1] &&
			o->bb.max[2] <= b1.max[2]
		) {
			//completely inside
			nobjects[0]++;
		}
		o = o->next;
	}
	//
	//Look for objects completely inside right box
	o = bvh->ohead;
	while(o) {
		if (o->bb.min[0] >= b2.min[0] &&
			o->bb.min[1] >= b2.min[1] &&
			o->bb.min[2] >= b2.min[2] &&
			o->bb.max[0] <= b2.max[0] &&
			o->bb.max[1] <= b2.max[1] &&
			o->bb.max[2] <= b2.max[2]
		) {
			//completely inside
			nobjects[1]++;
		}
		o = o->next;
	}
	//
	//What is the sum?
	//Log("nobjects: %i %i\n",nobjects[0],nobjects[1]);
	if (!nobjects[0] || !nobjects[1]) return 0;
	return (nobjects[0] + nobjects[1]);
}




