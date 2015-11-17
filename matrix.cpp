//cs371 Fall 2013
//program: matrix.cpp
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

void concatenateToMatrix(Transform *t)
{
	//Concatenate new transform with our existing matrix.
	if (g.transformHead != NULL) {
		//There is an existing matrix
		matrixCat(t->mat, g.transformHead->mat, t->mat);
	}
	//The stack is kept in a linked-list
	//Save the new transformation onto the stack
	t->next = g.transformHead;
	g.transformHead = t;
}

void translate(Transform *t)
{
	identity(t->mat);
	//Move translation into our new matrix
	//
	//      x     y     z
	//   |-----|-----|-----|-----|
	// x |  1  |     |     |     |
	//   |-----|-----|-----|-----|
	// y |     |  1  |     |     |
	//   |-----|-----|-----|-----|
	// z |     |     |  1  |     |
	//   |-----|-----|-----|-----|
	//   | tx  | ty  | tz  |  1  |
	//   |-----|-----|-----|-----|
	//
	t->mat[3][0] = t->translate[0];
	t->mat[3][1] = t->translate[1];
	t->mat[3][2] = t->translate[2];
}

void rotate(Transform *t)
{
	identity(t->mat);
	//Move rotation into our new matrix
	//This code is not optimized.
	if (t->rotate[0] != 0.0) {
		//
		//rotate on x-axis
		//
		//      x     y     z
		//   |-----|-----|-----|-----|
		// x |  1  |     |     |     |
		//   |-----|-----|-----|-----|
		// y |     | cos | sin |     |
		//   |-----|-----|-----|-----|
		// z |     |-sin | cos |     |
		//   |-----|-----|-----|-----|
		//   |     |     |     |  1  |
		//   |-----|-----|-----|-----|
		//
		t->mat[1][1] =  cos(t->rotate[0]);
		t->mat[1][2] =  sin(t->rotate[0]);
		t->mat[2][1] = -sin(t->rotate[0]);
		t->mat[2][2] =  cos(t->rotate[0]);
	}
	if (t->rotate[1] != 0.0) {
		//
		//rotate on y-axis
		//
		//      x     y     z
		//   |-----|-----|-----|-----|
		// x | cos |     |-sin |     |
		//   |-----|-----|-----|-----|
		// y |     |  1  |     |     |
		//   |-----|-----|-----|-----|
		// z | sin |     | cos |     |
		//   |-----|-----|-----|-----|
		//   |     |     |     |  1  |
		//   |-----|-----|-----|-----|
		//
		t->mat[0][0] =  cos(t->rotate[1]);
		t->mat[0][2] = -sin(t->rotate[1]);
		t->mat[2][0] =  sin(t->rotate[1]);
		t->mat[2][2] =  cos(t->rotate[1]);
	}
	if (t->rotate[2] != 0.0) {
		//
		//rotate on z-axis
		//
		//      x     y     z
		//   |-----|-----|-----|-----|
		// x | cos | sin |     |     |
		//   |-----|-----|-----|-----|
		// y |-sin | cos |     |     |
		//   |-----|-----|-----|-----|
		// z |     |     |  1  |     |
		//   |-----|-----|-----|-----|
		//   |     |     |     |  1  |
		//   |-----|-----|-----|-----|
		//
		t->mat[0][0] =  cos(t->rotate[2]);
		t->mat[0][1] =  sin(t->rotate[2]);
		t->mat[1][0] = -sin(t->rotate[2]);
		t->mat[1][1] =  cos(t->rotate[2]);
	}
}

void scale(Transform *t)
{
	identity(t->mat);
	//Move scale into our new matrix
	//
	//      x     y     z
	//   |-----|-----|-----|-----|
	// x | sx  |     |     |     |
	//   |-----|-----|-----|-----|
	// y |     | sy  |     |     |
	//   |-----|-----|-----|-----|
	// z |     |     | sz  |     |
	//   |-----|-----|-----|-----|
	//   |     |     |     |  1  |
	//   |-----|-----|-----|-----|
	//
	t->mat[0][0] = t->scale[0];
	t->mat[1][1] = t->scale[1];
	t->mat[2][2] = t->scale[2];
}


