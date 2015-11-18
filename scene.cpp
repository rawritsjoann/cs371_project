//cs371 Fall 2013
//program: scene.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: framework for ray tracing
//
//This file controls the reading of a script file that contains definitions
//for things like camera, light, objects, surfaces, animation, etc.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include "defs.h"
#include "log.h"
#include "vector.h"
#include "extern.h"
#include "ppm.h"
#include "animate.h"
#include "files.h"
extern struct Cloth cl;

const int MAXLINE = 200;
void initScene(void);
void fileloop(FILE *fpi);

//do looping
int doCount[8]={0,0,0,0,0,0,0,0};
int filespot[8];
int doNest=0;
//function calls
int funcFileSpot[8];
int funcNest=0;
//variables
typedef struct t_var {
	char name[32];
	Flt value;
	Flt *arr; //null if unused
	struct t_var *next;
} Variable;
Variable *vhead = NULL;


char *leftTrim(char *str)
{
	char *ptr = str;
	while (isspace(*ptr))
		ptr++;
	return ptr;
}

void initObject(Object *o, int type, void *listhead)
{
	memset(o, 0, sizeof(Object));
	o->type = type;
	o->obj = listhead;
	o->surface = g.surfaceHead;
}

void cleanupVariables()
{
	Variable *tmp, *v = vhead;
	while (v) {
		tmp = v->next;
		if (v->arr != NULL)
			free(v->arr);
		free(v);
		v = tmp;
	}
	vhead = NULL;
}

Variable *getVariablePtr(char *s)
{
	//search the linked list of variable names.
	Variable *v = vhead;
	while (v) {
		if (strcmp(s, v->name) == 0)
			break;
		v = v->next;
	}
	if (v == NULL) {
		//variable name not found.
		//create variables on-the-fly.
		//add this variable to linked list.
		v = (Variable *)malloc(sizeof(Variable));
		strcpy(v->name, s);
		v->value = 0.0;
		v->arr = NULL;
		v->next = vhead;
		vhead = v;
	}
	return v;
}

void getString(FILE *fpi, char *str) {
	if (fscanf(fpi, "%s", str)==0) {
		printf("Possible fscanf error!\n");
		fflush(stdout);
	}
}

Flt getVariableValue(FILE *fpi, char *vname)
{
	void getScalar(FILE *fpi, Flt *val);
	int spot;
	char token[16];
	void getString(FILE *fpi, char *str);
	void getInteger(FILE *fpi, int *ival);

	Variable *v = getVariablePtr(vname);
	//is this an array?
	spot = ftell(fpi);
	getString(fpi, token);
	if (*token == '[') {
		//this is an array value
		int idx;
		Flt fidx;
		getScalar(fpi, &fidx);
		idx = (int)fidx;
		getString(fpi, token);
		if (*token != ']') {
			printf("ERROR: array ] not found.\n");
			fflush(stdout);
		}
		return v->arr[idx];
	} else {
		fseek(fpi, spot, SEEK_SET);
		return v->value;
	}
}

void getScalar(FILE *fpi, Flt *val) {
	//printf("getScalar()...\n"); fflush(stdout);
	char ts[100], *s;
	getString(fpi, ts);
	s = ts;
	//check for minus sign
	Flt m = 1.0;
	if (*s == '-') {
		m = -1.0;
		s++;
	}
	if (isalpha(*s)) {
		//check reserved words...
		if (strcmp(s,"rnd")==0) {
			Flt r1,r2;
			getScalar(fpi, &r1);
			getScalar(fpi, &r2);
			*val = rnd() * r1 + r2;
		} else if (strcmp(s,"sin")==0) {
			Flt sval;
			getScalar(fpi, &sval);
			*val = sin(sval);
		} else if (strcmp(s,"cos")==0) {
			Flt sval;
			getScalar(fpi, &sval);
			*val = cos(sval);
		} else if (strcmp(s,"tan")==0) {
			Flt sval;
			getScalar(fpi, &sval);
			*val = tan(sval);
		} else {
			*val = getVariableValue(fpi, s);
		}
	} else {
		*val = atof(s);
	}
	*val *= m;
}

void getInteger(FILE *fpi, int *ival) {
	Flt val;
	getScalar(fpi, &val);
	*ival = (int)val;
}

void getVector(FILE *fpi, Vec v) {
	//parse next 3 tokens
	int i;
	for (i=0; i<3; i++)
		getScalar(fpi, &v[i]);
}

void setupArray(FILE *fpi)
{
	//array: col [ 3 ] == { 255 200 60 }
	int dim, i;
	char token[16], aname[32];
	//printf("setupArray()...\n"); fflush(stdout);
	getString(fpi, aname);
	getString(fpi, token);
	if (*token != '[') {
		printf("ERROR: missing [ in array setup.\n");
		fflush(stdout);
		return;
	}
	getInteger(fpi, &dim);
	getString(fpi, token);
	if (*token != ']') {
		printf("ERROR: missing ] in array setup.\n");
		fflush(stdout);
		return;
	}
	Variable *v = getVariablePtr(aname);
	//printf("dim: %i\n",dim);
	if (v->arr == NULL)
		v->arr = (Flt *)malloc(dim*sizeof(Flt)); 
	//-----------------------------------------------------------
	//array initialization is optional
	int spot = ftell(fpi);
	getString(fpi, token);
	if (*token != '=') {
		fseek(fpi, spot, SEEK_SET);
		return;
	}
	//continue initialization of array values
	getString(fpi, token);
	if (*token != '{') {
		printf("ERROR: missing { in array setup.\n");
		fflush(stdout);
		return;
	}
	//getString(fpi, token);
	i = 0;
	*token = '\0';
	while (*token != '}') {
		//printf("set value: %lf\n",atof(token));
		spot = ftell(fpi);
		getString(fpi, token);
		if (*token != '}') {
			fseek(fpi, spot, SEEK_SET);
			Flt f;
			getScalar(fpi, &f);
			v->arr[i++] = f;
		}
		//getString(fpi, token);
	}
}

void setArrayValue(FILE *fpi, char *s) {
	//           let: col [ 0 ] == .9
	//pointer is here -----^	
	//initialize an array of values
	char token[16], op[8];
	Flt val;
	//printf("setArrayValue()...\n");
	//next token is the index
	int idx;
	Flt fidx;
	getScalar(fpi, &fidx);
	idx = (int)fidx;
	//printf("arr value: %i\n",idx); fflush(stdout);
	//next token is ]
	getString(fpi, token);
	if (*token != ']') {
		printf("ERROR: missing ] in array variable.\n");
		fflush(stdout);
		return;
	}
	//get pointer to variable
	Variable *v = getVariablePtr(s);
	getString(fpi, op);
	getScalar(fpi, &val);
	int tmp;
	//printf("setting name **%s** idx: %i val: %lf\n",s,idx,val); fflush(stdout);
	if (op[1]=='=') {
		switch (op[0]) {
			case '=': v->arr[idx]  = val; break;
			case '+': v->arr[idx] += val; break;
			case '-': v->arr[idx] -= val; break;
			case '*': v->arr[idx] *= val; break;
			case '/': v->arr[idx] /= val; break;
			case '%':
				tmp = (int)v->arr[idx];
				tmp %= (int)val;
				v->arr[idx] = (Flt)tmp;
				break;
			case '^':
				tmp = (int)v->arr[idx];
				tmp ^= (int)val;
				v->arr[idx] = (Flt)tmp;
				break;
		}
	}
}

void setVar(FILE *fpi) {
	//let a == 123
	//let b += c
	int spot, tmp;
	char op[8];
	Flt val;
	char s[32], token[16];
	getString(fpi, s);
	//
	//is this an array?
	spot = ftell(fpi);
	getString(fpi, token);
	if (*token == '[') {
		setArrayValue(fpi, s);
		return;
	} else {
		fseek(fpi, spot, SEEK_SET);
	}
	//get pointer to variable
	Variable *v = getVariablePtr(s);
	//
	getString(fpi, op);
	getScalar(fpi, &val);
	if (op[1]=='=') {
		switch (op[0]) {
			case '=': v->value = val; break;
			case '+': v->value += val; break;
			case '-': v->value -= val; break;
			case '*': v->value *= val; break;
			case '/': v->value /= val; break;
			case '%':
				tmp = (int)v->value;
				tmp %= (int)val;
				v->value = (Flt)tmp;
				break;
			case '^':
				tmp = (int)v->value;
				tmp ^= (int)val;
				v->value = (Flt)tmp;
				break;
		}
	}
}

int evaluateBoolean(FILE *fpi)
{
	//There must be 3 components
	Flt operand1, operand2;
	char _operator[16];
	getScalar(fpi, &operand1);
	getString(fpi, _operator);
	getScalar(fpi, &operand2);
	//printf("if found   %lf %s %lf\n",operand1,operator,operand2);
	if (*_operator == '=')
		return (operand1 == operand2);
	if (strcmp(_operator,">")==0)
		return (operand1 > operand2);
	if (strcmp(_operator,"<")==0)
		return (operand1 < operand2);
	if (strcmp(_operator,">=")==0)
		return (operand1 >= operand2);
	if (strcmp(_operator,"<=")==0)
		return (operand1 <= operand2);
	if (strcmp(_operator,"!=")==0)
		return (operand1 != operand2);
	return 0;
}

void convertImage(char *originalName)
{
	//scene file had an image name other than .ppm
	//convert to ppm
	//the ppm file is temporary on disk.
	//save the ppm name in a list so that it is deleted when program ends.
	if (strstr(originalName,".ppm") != '\0')
		return;
	char ts[256];
	char ppmName[256];
	strcpy(ppmName, originalName);
	replaceFileExtension(ppmName, ".ppm");
	//If ppm does not exist already...
	if (access(ppmName, F_OK) != 0) {
		//call convert command
		sprintf(ts, "convert %s %s", originalName, ppmName);
		if (system(ts)) { printf("image error!\n"); };
		//Add a node to temporary ppm list.
		//List is stored so ppm's can be removed when program ends.
		int slen = strlen(ppmName);
		//build node
		TemporaryPPM *tppm =
					(TemporaryPPM *)malloc(sizeof(TemporaryPPM));
		tppm->ppmName = (char *)malloc(slen+10);
		strcpy(tppm->ppmName, ppmName);
		//link node to list
		tppm->next = g.temporaryPPMHead;
		g.temporaryPPMHead = tppm;
	}
	//printf("ppm ready **%s**\n",ppmName); fflush(stdout);
	strcpy(originalName, ppmName);
}

void transformScaling(Flt *value)
{
	//Log("transformScaling(value: %lf)...\n",*value);
	if (g.transformHead) {
		(*value) *= g.transformHead->mat[1][1];
		//int i,j;
		//for (i=0; i<3; i++) {
		//	for (j=0; j<3; j++) {
		//		Log("%lf",g.transformHead->mat[i][j]);
		//	}
		//	Log( ((j==2) ? "\n" : " ") );
		//}
	}
	//Log("new value: %lf\n",*value);
}

void doTransform(Vec v)
{
	Vec v1;
	if (g.transformHead) {
		transVector(g.transformHead->mat, v, v1);
		VecCopy(v1, v);
	}
}

void doTransformNorm(Vec v)
{
	//With a 4x4 matrix, we need a special function to transform a normal.
	Vec v1;
	if (g.transformHead) {
		transNormal(g.transformHead->mat, v, v1);
		VecCopy(v1, v);
	}
}

void bbCylinder(Cylinder *cy, Object *o)
{
	//Create bounding box for cylinder
	//
	//A cylinder is always transformed into place,
	//so build the bounding box based on transformed dimensions.
	//
	int j;
	Vec bottom = {0,0,0};
	Vec top = {0,cy->apex,0};
	if (g.transformHead) {
		transVector(g.transformHead->mat, bottom, bottom);
		transVector(g.transformHead->mat, top,    top);
	}
	//Initialize the bounding box
	for (j=0; j<3; j++) {
		o->bb.min[j] =  9e25;
		o->bb.max[j] = -9e25;
	}
	//Get bounding box dimensions
	Flt f;
	Flt rad = cy->radius + 0.5;
	for (j=0; j<3; j++) {
		f = bottom[j] - rad; if (o->bb.min[j] > f) o->bb.min[j] = f;
		f = bottom[j] + rad; if (o->bb.max[j] < f) o->bb.max[j] = f;
		f = top[j] - rad;    if (o->bb.min[j] > f) o->bb.min[j] = f;
		f = top[j] + rad;    if (o->bb.max[j] < f) o->bb.max[j] = f;
	}
}

void bbCone(Cone *cn, Object *o)
{
	//Create bounding box for cone
	o->bb.min[0] = (cn->center[0] - cn->radius);
	o->bb.max[0] = (cn->center[0] + cn->radius);
	o->bb.min[2] = (cn->center[2] - cn->radius);
	o->bb.max[2] = (cn->center[2] + cn->radius);
	o->bb.min[1] = (cn->center[1] + cn->base);
	o->bb.max[1] = (cn->center[1] + cn->apex);
}

void bbRing(Ring *r, Object *o)
{
	//Create bounding box for ring
	int j;
	for (j=0; j<3; j++) {
		o->bb.min[j] = (r->center[j] - r->maxradius);
		o->bb.max[j] = (r->center[j] + r->maxradius);
	}
}

void bbSphere(Sphere *s, Object *o)
{
	//Create bounding box for sphere
	int j;
	for (j=0; j<3; j++) {
		o->bb.min[j] = (s->center[j] - s->radius);
		o->bb.max[j] = (s->center[j] + s->radius);
	}
}

void bbTri(Tri *t, Object *o)
{
	//Create bounding box for triangle
	int i,j;
	MakeVector( 9e9, 9e9, 9e9,o->bb.min);
	MakeVector(-9e9,-9e9,-9e9,o->bb.max);
	for (i=0; i<3; i++) {
		for (j=0; j<3; j++) {
			if (o->bb.min[j] > t->verts[i][j])
				o->bb.min[j] = t->verts[i][j];
			if (o->bb.max[j] < t->verts[i][j])
				o->bb.max[j] = t->verts[i][j];
		}
	}
}

void swapVecs(Vec a, Vec b)
{
	//Swap vectors...
	Vec v;
	VecCopy(a,v);
	VecCopy(b,a);
	VecCopy(v,b);
}

void makeTriangle(Vec tv[3])
{
	int j;
	Tri *t = (Tri *)malloc(sizeof(Tri));
	t->next = g.triHead;
	for (j=0; j<3; j++) {
		VecCopy(tv[j], t->verts[j]);
		doTransform(t->verts[j]);
	}
	makeUnitNormalFromVerts(t->verts, t->norm);
	t->clip = NULL;
	if (g.nclips) {
		//put all clips into place
		int i;
		for (i=0; i<g.nclips; ++i) {
			Clip *c = (Clip *)malloc(sizeof(Clip));
			c->next = t->clip;
			t->clip = c;
			VecCopy(g.cliplist[i].center, t->clip->center);
			VecCopy(g.cliplist[i].norm, t->clip->norm);
			t->clip->radius = g.cliplist[i].radius;
		}
	}
	#ifdef ALLOW_PATCHES
	t->patch = 0;
	#endif //ALLOW_PATCHES
	//
	Object *o = (Object *)malloc(sizeof(Object));
	initObject(o, OBJECT_TYPE_TRI, t);
	o->intersectFunc = &rayTriIntersect;
	o->normalFunc = &triNormal;
	bbTri(t, o);
	g.triHead = t;
	o->next = g.objectHead;
	g.objectHead = o;
}


void buildModel(char *mname)
{
	int i;//j;
	char line[200];
	Vec *vert=NULL;
	int *face=NULL;
	int iface[4];
	int n, nv=0, nf=0;
	Vec tv[3];
	//Build a model exported from Blender.
	//Assume this is an obj file.
	FILE *fpi = fopen(mname,"r");
	if (!fpi) {
		printf("ERROR: file **%s** not found.\n", mname);
		return;
	}
	//
	//allocate mem
	//
	//count all vertices
	//
	fseek(fpi, 0, SEEK_SET);	
	while (fgets(line, 100, fpi) != NULL) {
		if (line[0] == 'v' && line[1] == ' ')
			nv++;
	}
	vert = (Vec *)malloc(nv * sizeof(Vec));
	if (!vert) {
		printf("ERROR: out of mem (vert)\n");
		exit(EXIT_FAILURE);		
	}
	printf("n verts: %i\n", nv);
	//
	//count all faces
	//
	fseek(fpi, 0, SEEK_SET);	
	while (fgets(line, 100, fpi) != NULL) {
		if (line[0] == 'f' && line[1] == ' ') {
			iface[0]=iface[1]=iface[2]=iface[3] = -1;
			sscanf(line+1,"%i %i %i %i", &iface[0],
										 &iface[1],
										 &iface[2],
										 &iface[3]);
			nf++;
			if (iface[3] >= 0)
				nf++;
		}
	}
	face = (int *)malloc(nf * sizeof(int) * 3);
	if (!face) {
		printf("ERROR: out of mem (face)\n");
		exit(EXIT_FAILURE);		
	}
	printf("n faces: %i\n", nf);
	//
	//first pass
	//read all vertices
	//
	nv=0;
	fseek(fpi, 0, SEEK_SET);	
	while (fgets(line, 100, fpi) != NULL) {
		if (line[0] == 'v' && line[1] == ' ') {
			sscanf(line+1,"%lf %lf %lf", &vert[nv][0],
										 &vert[nv][1],
										 &vert[nv][2]);
			nv++;
		}
	}
	//
	//second pass
	//read all faces
	//
	int comment=0;
	nf=0;
	fseek(fpi, 0, SEEK_SET);	
	while (fgets(line, 100, fpi) != NULL) {
		if (line[0] == '/' && line[1] == '*') {
			comment=1;
		}
		if (line[0] == '*' && line[1] == '/') {
			comment=0;
			continue;
		}
		if (comment)
			continue;
		if (line[0] == 'f' && line[1] == ' ') {
			iface[0]=iface[1]=iface[2]=iface[3] = -1;
			sscanf(line+1,"%i %i %i %i", &iface[0],
										 &iface[1],
										 &iface[2],
										 &iface[3]);
			n=3;
			if (iface[3] >= 0) n=4;
			face[nf*3+0] = iface[2];
			face[nf*3+1] = iface[1];
			face[nf*3+2] = iface[0];
			//Log("face: %i %i %i\n",face[nf][0],face[nf][1],face[nf][2]);
			nf++;
			if (n==4) {
				face[nf*3+0] = iface[3];
				face[nf*3+1] = iface[2];
				face[nf*3+2] = iface[0];
				//Log("face: %i %i %i\n",face[nf][0],face[nf][1],face[nf][2]);
				nf++;
			}
		}
	}
	fclose(fpi);
	
	int a,b,c;
/*	Vec norm;
	Vec normals[nv];
	Vec ttri[3];
	for (int i = 0; i < nv; i++) {
	    VecZero(normals[i]);
	    for (int j = 0; j < nf; j++) {
		a = face[i*3+0]-1;
		b = face[i*3+1]-1;
		c = face[i*3+2]-1;
		if (i == a || i == b || i == c) {
		    VecCopy(vert[a],ttri[0]);
		    VecCopy(vert[b],ttri[1]);
		    VecCopy(vert[c],ttri[2]);
		    makeUnitNormalFromVerts(ttri, norm);
		    normals[i][0] += norm[0];
		    normals[i][1] += norm[1];
		    normals[i][2] += norm[2];
		}
	    }
	    VecNormalize(normals[i]);
	}
*/
	//Log("nverts: %i   nfaces: %i\n", nv, nf);
	//
	//now build the triangles...
	for (i=0; i<nf; i++) {
		a = face[i*3+0]-1;
		tv[0][0] = vert[a][0];
		tv[0][1] = vert[a][1];
		tv[0][2] = vert[a][2];
		b = face[i*3+1]-1;
		tv[1][0] = vert[b][0];
		tv[1][1] = vert[b][1];
		tv[1][2] = vert[b][2];
		c = face[i*3+2]-1;
		tv[2][0] = vert[c][0];
		tv[2][1] = vert[c][1];
		tv[2][2] = vert[c][2];
		makeTriangle(tv);
/*		VecCopy(normals[a],g.triHead->pnorm[0]);
		VecCopy(normals[b],g.triHead->pnorm[1]);
		VecCopy(normals[c],g.triHead->pnorm[2]);
		g.triHead -> patch = 1;
*/	}
	free(vert);
	free(face);
}

void buildHeightmap(char *filename)
{
	int i,j;
	char ts[256];
	int width, height, max;
	int *irgb, *iptr;
	Vec tv[3];
	FILE *fpi;
	//Log("buildHeightmap()...\n");
	if (strlen(filename) < 5)
		return;
	fpi = fopen(filename,"r");
	if (!fpi) {
		Log("ERROR: file **%s** not found\n", filename);
		return;
	}
	//This must be in P3 format
	if (fgets(ts, 100, fpi) == NULL) {}
	if (fgets(ts, 100, fpi) == NULL) {}
	if (fscanf(fpi,"%i %i %i", &width, &height, &max) == 0) {}
	Log("heightmap width: %i %i %i\n",width,height,max);
	irgb = (int *)malloc(width*height*3*sizeof(int));
	//ptr = fgets((char *)rgb, width*height*3, fpi);
	iptr = irgb;
	for (i=0; i<width*height; i++) {
		for (j=0; j<3; j++) {
			if (fgets(ts, 10, fpi) == NULL) {}
			*iptr = atoi(ts);
			iptr++;
		}
	}
	for (i=0; i<height-1; i++) {
		for (j=0; j<width-1; j++) {
			tv[0][0]=(Flt)j;
			tv[1][0]=(Flt)j;
			tv[2][0]=(Flt)j+1;
			tv[0][1]=(Flt)*(irgb+((i+0)*width*3+j*3+0)) / 255.0 * 2.0;
			tv[1][1]=(Flt)*(irgb+((i+1)*width*3+j*3+0)) / 255.0 * 2.0;
			tv[2][1]=(Flt)*(irgb+((i+1)*width*3+j*3+1)) / 255.0 * 2.0;
			tv[0][2]=(Flt)i;
			tv[1][2]=(Flt)i+1;
			tv[2][2]=(Flt)i+1;
			swapVecs(tv[1],tv[2]);
			makeTriangle(tv);
			//
			tv[0][0]=(Flt)j;
			tv[1][0]=(Flt)j+1;
			tv[2][0]=(Flt)j+1;
			tv[0][1]=(Flt)*(irgb+((i+0)*width*3+j*3+0)) / 255.0 * 2.0;
			tv[1][1]=(Flt)*(irgb+((i+1)*width*3+j*3+1)) / 255.0 * 2.0;
			tv[2][1]=(Flt)*(irgb+((i+0)*width*3+j*3+1)) / 255.0 * 2.0;
			tv[0][2]=(Flt)i;
			tv[1][2]=(Flt)i+1;
			tv[2][2]=(Flt)i;
			swapVecs(tv[1],tv[2]);
			makeTriangle(tv);
		}
	}
	free(irgb);
	fclose(fpi);
}

void addBoxTri(Vec ct, Flt a, Flt b, Flt c, Flt d, Flt e, Flt f, Flt g,
															Flt h, Flt i)
{
	Vec tv[3];
	tv[0][0]=ct[0]+a;
	tv[1][0]=ct[0]+b;
	tv[2][0]=ct[0]+c;
	tv[0][1]=ct[1]+d;
	tv[1][1]=ct[1]+e;
	tv[2][1]=ct[1]+f;
	tv[0][2]=ct[2]+g;
	tv[1][2]=ct[2]+h;
	tv[2][2]=ct[2]+i;
	makeTriangle(tv);
}

void makeBox(Vec ct, Flt w, Flt h, Flt d, char *ts)
{
	//Log("make_box(%s)...\n",ts);
	//Build a box out of triangles
	//ts[] "LTRBFB" top,bottom,left,right,front,back
	Flt w2 = w * 0.5;
	Flt h2 = h * 0.5;
	Flt d2 = d * 0.5;
	if (ts[0]=='L') {
		addBoxTri(ct,-w2,-w2,-w2,-h2, h2, h2,-d2,-d2, d2);
		addBoxTri(ct,-w2,-w2,-w2,-h2, h2,-h2,-d2, d2, d2);
	}
	if (ts[1]=='T') {
		addBoxTri(ct,-w2,-w2, w2, h2, h2, h2, d2,-d2,-d2);
		addBoxTri(ct,-w2, w2, w2, h2, h2, h2, d2,-d2, d2);
	}
	if (ts[2]=='R') {
		addBoxTri(ct, w2, w2, w2,-h2, h2, h2, d2, d2,-d2);
		addBoxTri(ct, w2, w2, w2,-h2, h2,-h2, d2,-d2,-d2);
	}
	if (ts[3]=='B') {
		addBoxTri(ct,-w2,-w2, w2,-h2,-h2,-h2,-d2, d2, d2);
		addBoxTri(ct,-w2, w2, w2,-h2,-h2,-h2,-d2, d2,-d2);
	}
	if (ts[4]=='F') {
		addBoxTri(ct,-w2,-w2, w2,-h2, h2, h2, d2, d2, d2);
		addBoxTri(ct,-w2, w2, w2,-h2, h2,-h2, d2, d2, d2);
	}
	if (ts[5]=='B') {
		addBoxTri(ct,-w2,-w2, w2, h2,-h2,-h2,-d2,-d2,-d2);
		addBoxTri(ct,-w2, w2, w2, h2,-h2, h2,-d2,-d2,-d2);
	}
}

void buildSphereblob(Vec c, Vec b, int n)
{
	//Log("buildSphereblob(%lf %lf %lf %lf %lf %lf %i\n",
						//c[0],c[1],c[2],b[0],b[1],b[2],n);
	int i;
	Flt volume = b[0] * b[1] * b[2];
	// 4/3 * PI * r3
	Flt radius = (volume * (Flt)n * 3.0 / 4.0) / PI;
	radius = pow(radius, 0.3333333);
	radius *= 4.0;
	Log("radius: %lf\n", radius);
	radius /= (Flt)n;

	for (i=0; i<n; i++) {
		Sphere *s = (Sphere *)malloc(sizeof(Sphere));
		s->center[0] = c[0] + rnd() * b[0] - (b[0]/2.0);
		s->center[1] = c[1] + rnd() * b[1] - (b[1]/2.0);
		s->center[2] = c[2] + rnd() * b[2] - (b[2]/2.0);
		volume = b[0] * b[1] * b[2];
		s->radius = radius + rnd();
		//s->radius = radius;
		//s->radius = 1.0;
		s->r2 = s->radius * s->radius;
		doTransform(s->center);
		s->clip=NULL;
		if (g.nclips) {
			//put all clips into place
			int i;
			for (i=0; i<g.nclips; ++i) {
				Clip *c = (Clip *)malloc(sizeof(Clip));
				c->next = s->clip;
				s->clip = c;
				VecCopy(g.cliplist[i].center, s->clip->center);
				VecCopy(g.cliplist[i].norm, s->clip->norm);
				s->clip->radius = g.cliplist[i].radius;
			}
		}
		s->next = g.sphereHead;
		g.sphereHead = s;
		Object *o = (Object *)malloc(sizeof(Object));
		initObject(o, OBJECT_TYPE_SPHERE, g.sphereHead);
		o->intersectFunc = &raySphereIntersect;
		o->normalFunc = &sphereNormal;
		bbSphere(s, o);
		if (g.objectHead) g.objectHead->prev = o;
		o->next = g.objectHead;
		g.objectHead = o;
		g.objectHead->prev=NULL;
	}
}

//int check_for_comment(char *line)
//{
//	char *ptr=line;
//	while(isspace(*ptr)) ptr++;
//	if (*ptr == '#') return 1;
//	if (*ptr == '/') return 1;
//	return 0;
//}

void scene_a(char *line, FILE *fpi)
{
	if (strncasecmp(line, "at:", 3)==0) {
		getVector(fpi, g.studio.at);
		return;
	}
	if (strncasecmp(line, "adaptive:", 9)==0) {
		getScalar(fpi, &g.studio.var);
		g.studio.shoot |= SHOOT_ADAPTIVE;
		return;
	}
	if (strncasecmp(line, "ambient:", 8)==0) {
		getVector(fpi, g.studio.ambient);
		return;
	}
	if (strncasecmp(line, "ambient-occlusion:", 18)==0) {
		getInteger(fpi, &g.studio.ambientOcclusionN);
		getScalar(fpi, &g.studio.ambientOcclusionDist);
		return;
	}
	if (strncasecmp(line, "ambient-occlusion-func:", 23)==0) {
		getInteger(fpi, &g.studio.ambientOcclusionFunc);
		return;
	}
	if (strncasecmp(line, "aperture:", 9)==0) {
		getScalar(fpi, &g.studio.aperture.radius);
		getScalar(fpi, &g.studio.aperture.focalLength);
		getInteger(fpi, &g.studio.aperture.nsamples);
		g.studio.aperture.r5 = g.studio.aperture.radius * 0.5;
		g.studio.shoot |= SHOOT_APERTURE;
		return;
	}
	if (strncasecmp(line, "aperture-radius:", 16)==0) {
		getScalar(fpi, &g.studio.aperture.radius);
		g.studio.aperture.r5 = g.studio.aperture.radius * 0.5;
		g.studio.shoot |= SHOOT_APERTURE;
		return;
	}
	if (strncasecmp(line, "aperture-focal-length:", 22)==0) {
		getScalar(fpi, &g.studio.aperture.focalLength);
		g.studio.shoot |= SHOOT_APERTURE;
		return;
	}
	if (strncasecmp(line, "aperture-nsamples:", 18)==0) {
		getInteger(fpi, &g.studio.aperture.nsamples);
		g.studio.shoot |= SHOOT_APERTURE;
		return;
	}
	if (strncasecmp(line, "angle:", 6)==0) {
		getScalar(fpi, &g.studio.angle);
		return;
	}
	if (strncasecmp(line, "array:", 6)==0) {
		//setup an array, maybe initialize the array.
		setupArray(fpi);
		return;
	}
}

void scene_b(char *line, FILE *fpi)
{
	if (strncasecmp(line, "background:", 11)==0) {
		getVector(fpi, g.studio.background);
		return;
	}
	if (strncasecmp(line, "box:", 4)==0) {
		Vec c={0,0,0};
		Flt w=0,h=0,d=0;
		char ts[16];
		getString(fpi, ts);
		getVector(fpi, c);
		getScalar(fpi, &w);
		getScalar(fpi, &h);
		getScalar(fpi, &d);
		makeBox(c,w,h,d,ts);
		return;
	}
}

void removeCRLF(char *str) {
	char *ptr = str;
	while (*ptr) {
		if (*ptr == 10 || *ptr == 13) {
			*ptr = '\0';
			break;
		}
		ptr++;
	}
}

void scene_c(char *line, FILE *fpi)
{
	Vec scale={0,0,0};
	Vec col0={0,0,0};
	Vec col1={0,0,0};

	if (strncasecmp(line, "cloth-", 6)==0) {
		if (strncasecmp(line, "cloth-center:", 13)==0) {
			getVector(fpi, cl.clothCenter);
			return;
		}
		if (strncasecmp(line, "cloth-velocity:", 15)==0) {
			getVector(fpi, cl.clothVelocity);
			return;
		}
		if (strncasecmp(line, "cloth-width:", 12)==0) {
			getInteger(fpi, &cl.clothWidth);
			return;
		}
		if (strncasecmp(line, "cloth-height:", 13)==0) {
			getInteger(fpi, &cl.clothHeight);
			return;
		}
		if (strncasecmp(line, "cloth-step:", 11)==0) {
			getScalar(fpi, &cl.clothStep);
			return;
		}
		if (strncasecmp(line, "cloth-stiffness:", 16)==0) {
			getScalar(fpi, &cl.stiffness);
			return;
		}
		if (strncasecmp(line, "cloth-stretchiness:", 19)==0) {
			getScalar(fpi, &cl.stretchiness);
			return;
		}
		if (strncasecmp(line, "cloth-grabbed:", 14)==0) {
			int tmp;
			getInteger(fpi, &tmp);
			cl.clothGrabbed[cl.clothNgrabbed++] = tmp;
			return;
		}
		if (strncasecmp(line, "cloth-gravity:", 14)==0) {
			getScalar(fpi, &cl.clothGravity);
			//printf("cl.clothGravity: %lf\n",cl.clothGravity);
			return;
		}
		if (strncasecmp(line, "cloth-damping:", 14)==0) {
			getScalar(fpi, &cl.clothDamping);
			return;
		}
		if (strncasecmp(line, "cloth-spring-steps:", 20)==0) {
			getInteger(fpi, &cl.clothSpringSteps);
			//printf("cl.clothSpringSteps: %i\n",cl.clothSpringSteps);
			return;
		}
		if (strncasecmp(line, "cloth-texture:", 14)==0) {
			getString(fpi, cl.clothTexture);
			return;
		}
		if (strncasecmp(line, "cloth-tex-tile:", 15)==0) {
			getScalar(fpi, &cl.clothTexTile[0]);
			getScalar(fpi, &cl.clothTexTile[1]);
			return;
		}
		if (strncasecmp(line, "cloth-smoothing:", 16)==0) {
			cl.clothSmoothing = 1;
			return;
		}
	}
	if (strncasecmp(line, "call:", 5)==0) {
		char funcname[100];
		getString(fpi, funcname);
		//printf("calling object **%s**\n", obname);
		//save current position in file
		int spot = ftell(fpi);
		funcFileSpot[funcNest] = spot;
		funcNest++;
		//printf("saved filespot: %i\n", spot);
		//jump to function in .b file
		//printf("jump to function...\n");
		char xline[200];
		char test[200];
		sprintf(test, "function: %s", funcname);
		if (fgets(xline, 200, fpi)) {}
		removeCRLF(xline);
		while (!feof(fpi)) {
			//printf("compare **%s** **%s**\n", xline, test);
			if (strcmp(xline, test)==0) {
				//found object start.
				//printf("found object start.\n");
				break;
			}
			if (fgets(xline, 200, fpi)) {}
			removeCRLF(xline);
		}
		return;
	}
	if (strncasecmp(line, "checker:", 8)==0) {
		getVector(fpi, scale);
		getVector(fpi, col0);
		getVector(fpi, col1);
		VecCopy(scale, g.cpattern[g.ncpatterns].scale);
		VecCopy(col0, g.cpattern[g.ncpatterns].color0);
		VecCopy(col1, g.cpattern[g.ncpatterns].color1);
		g.cpattern[g.ncpatterns].grout=0;
		g.ncpatterns++;
		return;
	}
	if (strncasecmp(line, "checker-grout:", 14)==0) {
		//printf("found grout\n");
		int j;
		Vec col={0,0,0};
		Flt w;
		getVector(fpi, col);
		getScalar(fpi, &w);
		j = g.ncpatterns-1;
		VecCopy(col, g.cpattern[j].color2);
		g.cpattern[j].groutWidth = w;
		g.cpattern[j].grout=1;
		return;
	}
	if (strncasecmp(line, "clip:", 5)==0) {
		Clip c;
		getVector(fpi, c.center);
		getVector(fpi, c.norm);
		if (g.nclips < MAXCLIPS) {
			VecNormalize(c.norm);
			//save untransformed clip
			VecCopy(c.center, c.ucenter);
			VecCopy(c.norm, c.unorm);
			c.radius = 0.0;
			memcpy(&g.cliplist[g.nclips], &c, sizeof(Clip));
			//now transform
			doTransform(g.cliplist[g.nclips].center);
			doTransformNorm(g.cliplist[g.nclips].norm);
			//Log("unorm: %lf %lf %lf\n",
			//g.cliplist[g.nclips].unorm[0],
			//g.cliplist[g.nclips].unorm[1],
			//g.cliplist[g.nclips].unorm[2]);
			g.nclips++;
		}
		return;
	}
	//if (strncasecmp(line, "clip-normal:", 12)==0) {
	//	getVector(fpi, g.cliplist[g.nclips].norm);
	//	g.nclips++;
	//	return;
	//}
	if (strncasecmp(line, "clip-radius:", 12)==0) {
		getScalar(fpi, &g.cliplist[g.nclips-1].radius);
		VecZero(g.cliplist[g.nclips-1].norm);
		return;
	}
	if (strncasecmp(line, "clip-pop:", 9)==0) {
		if (--g.nclips < 0)
			g.nclips = 0;
		//Parameter is optional. Might be blank.
		//Default pops is 1
		int npops;
		int spot = ftell(fpi);
		Flt ftmp;
		getScalar(fpi, &ftmp);
		npops = (int)ftmp;
		if (npops==0) {
			npops=1;
			fseek(fpi, spot, SEEK_SET);
		}
		while(npops > 0) {
			//Even if user enters large number of pops,
			//the stack integrity is safe.
			if (--g.nclips < 0)
				g.nclips = 0;
			--npops;
		}
		return;
	}
	if (strncasecmp(line, "cylinder:", 9)==0) {
		Cylinder *cy = (Cylinder *)malloc(sizeof(Cylinder));
		getScalar(fpi, &cy->radius);
		getScalar(fpi, &cy->apex);
		cy->clip=NULL;
		if (g.nclips) {
			//put all clips into place
			int i;
			for (i=0; i<g.nclips; ++i) {
				Clip *c = (Clip *)malloc(sizeof(Clip));
				c->next = cy->clip;
				cy->clip = c;
				VecCopy(g.cliplist[i].center, cy->clip->center);
				VecCopy(g.cliplist[i].norm, cy->clip->norm);
				VecCopy(g.cliplist[i].ucenter, cy->clip->ucenter);
				VecCopy(g.cliplist[i].unorm, cy->clip->unorm);
				cy->clip->radius = g.cliplist[i].radius;
			}
		}
		//Add to cylinder linked list
		cy->next = g.cylinderHead;
		g.cylinderHead = cy;
		//build an object structure
		Object *o = (Object *)malloc(sizeof(Object));
		initObject(o, OBJECT_TYPE_CYLINDER, g.cylinderHead);
		o->intersectFunc = &rayCylinderIntersect;
		o->normalFunc = &cylinderNormal;
		bbCylinder(cy, o);
		o->next = g.objectHead;
		g.objectHead = o;
		return;
	}
	if (strncasecmp(line, "cylinder-from-to:", 17)==0) {
		return;
	}
	if (strncasecmp(line, "cone:", 5)==0) {
		Cone *cn = (Cone *)malloc(sizeof(Cone));
		getVector(fpi, cn->center);
		getScalar(fpi, &cn->radius);
		getScalar(fpi, &cn->base);
		getScalar(fpi, &cn->apex);
		doTransform(cn->center);
		cn->clip=NULL;
		if (g.nclips) {
			//put all clips into place
			int i;
			for (i=0; i<g.nclips; ++i) {
				Clip *c = (Clip *)malloc(sizeof(Clip));
				c->next = cn->clip;
				cn->clip = c;
				VecCopy(g.cliplist[i].center, cn->clip->center);
				VecCopy(g.cliplist[i].norm, cn->clip->norm);
				cn->clip->radius = g.cliplist[i].radius;
			}
		}
		cn->next = g.coneHead;
		g.coneHead = cn;
		Object *o = (Object *)malloc(sizeof(Object));
		initObject(o, OBJECT_TYPE_CONE, g.coneHead);
		o->intersectFunc = &rayConeIntersect;
		o->normalFunc = &coneNormal;
		bbCone(cn, o);
		o->next = g.objectHead;
		g.objectHead = o;
		return;
	}
	if (strncasecmp(line, "cubemap:", 8)==0) {
		//Log("cubemap:\n");
		char fname[128];
		getString(fpi, fname);
		//Log("old fname **%s**\n",fname);
		convertImage(fname);
		//Log("new fname **%s**\n",fname);
		buildCubemapTexture(fname);
		g.cubemap.active=1;
		return;
	}
}

void scene_d(char *line, FILE *fpi)
{
	if (strncasecmp(line, "directory:", 10)==0) {
		getString(fpi, g.anim.directory);
		//Does directory exist?
		struct stat s;
		int err = stat(g.anim.directory, &s);
		if (-1 == err) {
			if (ENOENT == errno) {
				//does not exist
				printf("creating dir %s\n",g.anim.directory);
				mkdir(g.anim.directory, 0700);
			} else {
				perror("stat");
				exit(1);
			}
		} else {
			if (S_ISDIR(s.st_mode)) {
				//printf("dir found.\n");
			} else {
				//exists but is no dir
				printf("Please check dir name.\n");
			}
		}
		return;
	}
	if (strncasecmp(line, "distribution:", 13)==0) {
		//g.studio.distribution=1;
		g.studio.shoot |= SHOOT_DISTRIBUTE4;
		return;
	}
	if (strncasecmp(line, "do:", 3)==0) {
		Flt x;
		getScalar(fpi, &x);
		doNest++;
		doCount[doNest] = (int)x;
		filespot[doNest] = ftell(fpi);
		//printf("docount: %i\n",docount); fflush(stdout);
		//printf("filespot: %i\n",filespot); fflush(stdout);
		return;
	}
}

void scene_e(char *line, FILE *fpi)
{
	if (strncasecmp(line, "else:", 5)==0) {
		//skip to endif:
		char xline[200];
		if (fgets(xline, 200, fpi)) {}
		while (!feof(fpi)) {
			char *ptr = leftTrim(xline);
			if (strncasecmp(ptr, "endif:", 6)==0)
				break;
			if (fgets(xline, 200, fpi)) {}
		}
		return;
	}
	if (strncasecmp(line, "enddo:", 6)==0) {
		//printf("doCount[doNest]: %i\n",doCount[doNest]); fflush(stdout);
		if (--doCount[doNest] > 0) {
			fseek(fpi, filespot[doNest], SEEK_SET);
			//printf("goto docount: %i\n",docount); fflush(stdout);
			//printf("goto filespot: %i\n",filespot); fflush(stdout);
		} else {
			doNest--;
		}
		return;
	}
	if (strncasecmp(line, "endfunction:", 12)==0) {
		//printf("found endfunction:\n");
		if (funcNest > 0) {
			funcNest--;
			fseek(fpi, funcFileSpot[funcNest], SEEK_SET);
		}
		return;
	}
}

void scene_f(char *line, FILE *fpi)
{
	if (strncasecmp(line, "frameMod:", 9)==0) {
		getInteger(fpi, &g.anim.frameMod);
		//printf("g.anim.frameMod: %i\n", g.anim.frameMod);
		return;
	}
	if (strncasecmp(line, "frameStill:", 11)==0) {
		getInteger(fpi, &g.anim.frameStill);
		//printf("g.anim.frameStill: %i\n", g.anim.frameStill);
		return;
	}
	if (strncasecmp(line, "frameStop:", 10)==0) {
		getInteger(fpi, &g.anim.frameStop);
		return;
	}
	if (strncasecmp(line, "frameStart:", 11)==0) {
		getInteger(fpi, &g.anim.frameStart);
		return;
	}
	if (strncasecmp(line, "from:", 5)==0) {
		getVector(fpi, g.studio.from);
		return;
	}
	if (strncasecmp(line, "function:", 9)==0) {
		char xline[200];
		//getString(fpi, xline);
		//printf("skip object **%s**\n", xline);
		//skip object. must be called with call: --obj--
		//jump to endobject in .b file
		if (fgets(xline, 200, fpi)) {}
		while (!feof(fpi)) {
			char *ptr = leftTrim(xline);
			if (strncasecmp(ptr, "endfunction:", 10)==0) {
				//printf("found endfunction.\n");
				break;
			}
			if (fgets(xline, 200, fpi)) {}
		}
		return;
	}
}

void scene_h(char *line, FILE *fpi)
{
	if (strncasecmp(line, "haze:", 5)==0) {
		Haze *h = (Haze *)malloc(sizeof(Haze));
		getVector(fpi, h->color);
		getScalar(fpi, &h->density);
		h->type = HAZE_TYPE_BORDERS;
		h->fog = 0;
		h->linear = 0.0;
		h->media = 0;
		h->mediaStep = 1.0;
		h->mediaDensity = 0.001;
		h->mediaAdaptive = 0;
		h->next = g.hazeHead;
		g.hazeHead = h;
		//Log("found haze\n");
		g.studio.hazeOn=1;
		return;
	}
	if (strncasecmp(line, "haze-center:", 12)==0) {
		if (g.hazeHead)
			getVector(fpi, g.hazeHead->center);
		return;
	}
	if (strncasecmp(line, "haze-reach:", 11)==0) {
		if (g.hazeHead)
			getVector(fpi, g.hazeHead->reach);
		return;
	}
	if (strncasecmp(line, "haze-media:", 11)==0) {
		if (g.hazeHead)
			g.hazeHead->media = 1;
		return;
	}
	if (strncasecmp(line, "haze-media-step:", 16)==0) {
		if (g.hazeHead)
			getScalar(fpi, &g.hazeHead->mediaStep);
		return;
	}
	if (strncasecmp(line, "haze-media-density:", 19)==0) {
		if (g.hazeHead)
			getScalar(fpi, &g.hazeHead->mediaDensity);
		return;
	}
	if (strncasecmp(line, "haze-media-adaptive:", 20)==0) {
		if (g.hazeHead)
			g.hazeHead->mediaAdaptive = 1;
		return;
	}
	if (strncasecmp(line, "haze-fog:", 9)==0) {
		if (g.hazeHead)
			g.hazeHead->fog = 1;
		return;
	}
	if (strncasecmp(line, "haze-linear:", 12)==0) {
		if (g.hazeHead)
			getScalar(fpi, &g.hazeHead->linear);
		return;
	}
	if (strncasecmp(line, "heightmap:", 10)==0) {
		char filename[128];
		getString(fpi, filename);
		buildHeightmap(filename);
		return;
	}
}

void scene_i(char *line, FILE *fpi)
{
	if (strncasecmp(line, "if:", 3)==0) {
		if (evaluateBoolean(fpi)) {
			//continue as normal...
		} else {
			//search for else: or endif:
			char xline[200];
			if (fgets(xline, 200, fpi)) {}
			while (!feof(fpi)) {
				char *ptr = leftTrim(xline);
				if (strncasecmp(ptr, "else:", 5)==0) {
					//printf("skipped to else:\n");
					break;
				}
				if (strncasecmp(ptr, "endif:", 6)==0)
					break;
				if (fgets(xline, 200, fpi)) {}
			}
		}
		return;
	}
}

void scene_j(char *line, FILE *fpi)
{
	if (strncasecmp(line, "jitter:", 7)==0) {
		getScalar(fpi, &g.studio.jval);
		g.studio.jval2 = g.studio.jval * 0.5;
		g.studio.jitter=1;
		//g.studio.shoot |= SHOOT_JITTER;
		return;
	}
}


void scene_l(char *line, FILE *fpi)
{
	if (strncasecmp(line, "let:", 4)==0) {
		//let: a == 0
		setVar(fpi);
		return;
	}
	if (strncasecmp(line, "light:", 6)==0) {
		Light *l = (Light *)malloc(sizeof(Light));
		//check for light center
		//VecZero(l->center);
		getVector(fpi, l->center);
		doTransform(l->center);
		//initialize
		l->specular=0;
		l->area=0;
		l->nphotons=0;
		l->cache_obj=NULL;
		l->nphotons = 0;
		l->photonPower = 1.0;
		l->falloff = 0;
		l->falloffVal = 1.0;
		//Add light to linked list
		l->next = g.lightHead;
		g.lightHead = l;
		return;
	}
	if (strncasecmp(line, "light-", 6)==0) {
		if (strncasecmp(line, "light-center:", 13)==0) {
			getVector(fpi, g.lightHead->center);
			doTransform(g.lightHead->center);
			return;
		}
		if (strncasecmp(line, "light-color:", 12)==0) {
			getVector(fpi, g.lightHead->color);
			return;
		}
		if (strncasecmp(line, "light-spec:", 11)==0) {
			getInteger(fpi, &g.lightHead->specular);
			return;
		}
		if (strncasecmp(line, "light-area:", 11)==0) {
			getScalar(fpi, &g.lightHead->radius);
			getInteger(fpi, &g.lightHead->nsamples);
			g.lightHead->area=1;
			return;
		}
		if (strncasecmp(line, "light-photons:", 14)==0) {
			getInteger(fpi, &g.lightHead->nphotons);
			g.nphotons += g.lightHead->nphotons;
			return;
		}
		if (strncasecmp(line, "light-photonPower:", 18)==0) {
			getScalar(fpi, &g.lightHead->photonPower);
			return;
		}
		if (strncasecmp(line, "light-photonSeeThem:", 20)==0) {
			g.photonSeeThem = 1;
			return;
		}
		if (strncasecmp(line, "light-photonShadows:", 20)==0) {
			g.photonShadows = 1;
			return;
		}
		//if (strncasecmp(line, "light-photonDir:", 16)==0) {
		//	sscanf(line+16,"%i %i %i", &g.lightHead->photonDir);
		//	return;
		//}
		if (strncasecmp(line, "light-falloff:", 14)==0) {
			getInteger(fpi, &g.lightHead->falloff);
			getScalar(fpi, &g.lightHead->falloffVal);
			if (g.lightHead->falloffVal == 0.0)
				g.lightHead->falloffVal = 1.0;
			g.lightHead->oofalloffVal = 1.0 / g.lightHead->falloffVal;
			return;
		}
	}
}

void scene_m(char *line, FILE *fpi)
{
	if (strncasecmp(line, "maxdepth:", 9)==0) {
		getInteger(fpi, &g.studio.maxDepth);
		return;
	}
	if (strncasecmp(line, "minweight:", 10)==0) {
		getScalar(fpi, &g.studio.minWeight);
		return;
	}
	if (strncasecmp(line, "montecarlo:", 11)==0) {
		getInteger(fpi, &g.studio.nsamples);
		g.studio.shoot |= SHOOT_MONTECARLO;
		return;
	}
	if (strncasecmp(line, "model:", 6)==0) {
		char mname[256];
		getString(fpi, mname);
		buildModel(mname);
		return;
	}
}

void scene_n(char *line, FILE *fpi)
{
	if (strncasecmp(line, "nframes:", 8)==0) {
		getInteger(fpi, &g.anim.nframes);
		//printf("just read ---> g.anim.nframes: %i\n", g.anim.nframes);
		return;
	}
}

//void scene_o(char *line, FILE *fpi)
//{
//}


void scene_p(char *line, FILE *fpi)
{
	if (strncasecmp(line, "patch:", 6)==0) {
		Vec tv[3];
		Vec no[3];
		//printf("pat\n");
		getVector(fpi, tv[0]);
		getVector(fpi, no[0]);
		getVector(fpi, tv[1]);
		getVector(fpi, no[1]);
		getVector(fpi, tv[2]);
		getVector(fpi, no[2]);
		makeTriangle(tv);
	//	for (int i=0; i<3; i++) {
	//		VecNegate(no[i]);
	//		VecCopy(no[i], g.triHead->pnorm[i]);
	//		VecNormalize(g.triHead->pnorm[i]);
	//	}
	//	g.triHead->patch = 1;
		return;
	}
}


void scene_r(char *line, FILE *fpi)
{
	if (strncasecmp(line, "ring:", 5)==0) {
		Ring *r = (Ring *)malloc(sizeof(Ring));
		r->minradius=0.0;
		getVector(fpi, r->center);
		getVector(fpi, r->norm);
		getScalar(fpi, &r->minradius);
		getScalar(fpi, &r->maxradius);
		doTransform(r->center);
		doTransformNorm(r->norm);
		r->clip=NULL;
		if (g.nclips) {
			//put all clips into place
			int i;
			for (i=0; i<g.nclips; ++i) {
				Clip *c = (Clip *)malloc(sizeof(Clip));
				c->next = r->clip;
				r->clip = c;
				VecCopy(g.cliplist[i].center, r->clip->center);
				VecCopy(g.cliplist[i].norm, r->clip->norm);
				r->clip->radius = g.cliplist[i].radius;
			}
		}
		VecNormalize(r->norm);
		r->next = g.ringHead;
		g.ringHead = r;
		Object *o = (Object *)malloc(sizeof(Object));
		initObject(o, OBJECT_TYPE_RING, g.ringHead);
		o->intersectFunc = &rayRingIntersect;
		o->normalFunc = &ringNormal;
		bbRing(r, o);
		o->next = g.objectHead;
		g.objectHead = o;
		return;
	}
	if (strncasecmp(line, "row-end:", 8)==0) {
		getInteger(fpi, &g.studio.row_end);
		return;
	}
	if (strncasecmp(line, "row-start:", 10)==0) {
		getInteger(fpi, &g.studio.row_start);
		printf("g.studio.row_start: %i\n", g.studio.row_start);
		return;
	}
}

void scene_s(char *line, FILE *fpi)
{
	if (strncasecmp(line, "saveFiles:", 10)==0) {
		g.anim.saveFiles = 1;
		return;
	}
	if (strncasecmp(line, "sky:", 4)==0) {
		g.studio.sky.type = SKY_TYPE_BLUE;
		return;
	}
	if (strncasecmp(line, "sky-cube:", 9)==0) {
		g.studio.sky.type = SKY_TYPE_ENVIRONMENT;
		return;
	}
	if (strncasecmp(line, "sky-perlin:", 11)==0) {
		getVector(fpi, g.studio.sky.color[0]);
		getVector(fpi, g.studio.sky.color[1]);
		getVector(fpi, g.studio.sky.scale);
		getScalar(fpi, &g.studio.sky.horizon);
		g.studio.sky.type = SKY_TYPE_PERLIN;
		return;
	}
	if (strncasecmp(line, "surf:", 5)==0) {
		Surface *sf = (Surface *)malloc(sizeof(Surface));
		VecZero(sf->diffuse);
		//diffuse is optional on this input
		getVector(fpi, sf->diffuse);
		sf->transmitYN = 0;
		sf->specularYN = 0;
		sf->highlightYN = 0;
		sf->emitYN = 0;
		sf->pattern = 0;
		sf->photonsYN = 1;
		sf->translucent.YN = 0;
		sf->insideout = 0;
		sf->sub = 0;
		sf->tm = NULL;
		sf->spherical = 0;
		sf->cylindrical = 0;
		//save the transformation matrix and its inverse
		identity(sf->mat);
		identity(sf->invmat);
		if (g.transformHead) {
			Matrix m;
			matrixCopy(g.transformHead->mat, sf->mat);
			matrixInverse(g.transformHead->mat, m);
			matrixCopy(m, sf->invmat);
		}
		//add to linked list
		sf->next = g.surfaceHead;
		g.surfaceHead = sf;
		return;
	}
	if (strncasecmp(line, "surf-", 5)==0) {
		if (strncasecmp(line, "surf-perlin:", 12)==0) {
			Vec c0,c1,sc;
			getVector(fpi, c0);
			getVector(fpi, c1);
			getVector(fpi, sc);
			VecCopy(c0, g.surfaceHead->perlin_color[0]);
			VecCopy(c1, g.surfaceHead->perlin_color[1]);
			g.surfaceHead->perlin_scale[0] = 1.0 / sc[0];
			g.surfaceHead->perlin_scale[1] = 1.0 / sc[1];
			g.surfaceHead->perlin_scale[2] = 1.0 / sc[2];
			return;
		}
		if (strncasecmp(line, "surf-diffuse:", 13)==0) {
			getVector(fpi, g.surfaceHead->diffuse);
			return;
		}
		if (strncasecmp(line, "surf-specular:", 14)==0) {
			getVector(fpi, g.surfaceHead->specular);
			g.surfaceHead->specularYN=1;
			return;
		}
		if (strncasecmp(line, "surf-transmit:", 14)==0) {
			getVector(fpi, g.surfaceHead->transmit);
			VecS(0.001, g.surfaceHead->transmit, g.surfaceHead->transmit);
			g.surfaceHead->transmitYN=1;
			if (g.surfaceHead->transmit[0]+
				g.surfaceHead->transmit[1]+
				g.surfaceHead->transmit[2] > 0.0) {
				//Log("transmit=2\n");
				g.surfaceHead->transmitYN=2;
			}
			return;
		}
		if (strncasecmp(line, "surf-iors:", 10)==0 ||
		    strncasecmp(line, "surf-iors-in-out:", 17)==0) {
			g.surfaceHead->iorInside = 1.0;
			g.surfaceHead->iorOutside = 1.0;
			getScalar(fpi, &g.surfaceHead->iorInside);
			getScalar(fpi, &g.surfaceHead->iorOutside);
			if (g.surfaceHead->iorOutside == 0.0)
				g.surfaceHead->iorOutside = 1.0;
			return;
		}
		if (strncasecmp(line, "surf-iors-out-in:", 17)==0) {
			g.surfaceHead->iorInside = 1.0;
			g.surfaceHead->iorOutside = 1.0;
			getScalar(fpi, &g.surfaceHead->iorOutside);
			getScalar(fpi, &g.surfaceHead->iorInside);
			if (g.surfaceHead->iorOutside == 0.0)
				g.surfaceHead->iorOutside = 1.0;
			return;
		}
		if (strncasecmp(line, "surf-highlight:", 15)==0) {
			getVector(fpi, g.surfaceHead->highlight);
			g.surfaceHead->highlightYN=1;
			return;
		}
		if (strncasecmp(line, "surf-emit:", 10)==0) {
			//For flourescent paint
			getVector(fpi, g.surfaceHead->emit);
			g.surfaceHead->emitYN=1;
			return;
		}
		if (strncasecmp(line, "surf-noPhotons:", 15)==0) {
			g.surfaceHead->photonsYN=0;
			return;
		}
		if (strncasecmp(line, "surf-spot:", 10)==0) {
			getScalar(fpi, &g.surfaceHead->spot);
			return;
		}
		if (strncasecmp(line, "surf-spherical:", 15)==0) {
			g.surfaceHead->spherical = 1;
			//printf("spherical\n");
			return;
		}
		if (strncasecmp(line, "surf-cylindrical:", 17)==0) {
			g.surfaceHead->cylindrical = 1;
			//printf("cylinder\n");
			return;
		}
		if (strncasecmp(line, "surf-pattern:", 13)==0) {
			getInteger(fpi, &g.surfaceHead->pattern);
			if (g.surfaceHead->pattern == 1)
				g.surfaceHead->checker_idx = g.ncpatterns-1;
			return;
		}
		if (strncasecmp(line, "surf-translucent:", 17)==0) {
			getScalar(fpi, &g.surfaceHead->translucent.val);
			getInteger(fpi, &g.surfaceHead->translucent.nsamples);
			g.surfaceHead->translucent.YN=1;
			return;
		}
		if (strncasecmp(line, "surf-texmap:", 12)==0) {
			char fname[256];
			getString(fpi, fname);
			g.surfaceHead->tm = (Texmap *)malloc(sizeof(Texmap));
			//Is this NOT a ppm image??? Then convert.
			convertImage(fname);
			Ppmimage *image = ppm6GetImage(fname);
			g.surfaceHead->tm->xres = image->width;
			g.surfaceHead->tm->yres = image->height;
			g.surfaceHead->tm->data = (unsigned char *)image->data;
			return;
		}
		if (strncasecmp(line, "surf-insideout:", 15)==0) {
			g.surfaceHead->insideout = 1;
			return;
		}
		if (strncasecmp(line, "surf-sub:", 9)==0) {
			getInteger(fpi, &g.surfaceHead->sub);
			return;
		}
		if (strncasecmp(line, "surf-copy:", 10)==0) {
			Surface *sf = (Surface *)malloc(sizeof(Surface));
			memcpy(sf, g.surfaceHead, sizeof(Surface));
			sf->insideout = 0;
			sf->tm = NULL;
			//save the transformation matrix and its inverse
			identity(sf->mat);
			identity(sf->invmat);
			if (g.transformHead) {
				Matrix m;
				//matrixCopy(transformHead->mat, sf->mat);
				memcpy(sf->mat, g.transformHead->mat, sizeof(Matrix));
				matrixInverse(g.transformHead->mat, m);
				//matrixCopy(m, sf->invmat);
				memcpy(sf->invmat, m, sizeof(Matrix));
			}
			//add to linked list
			sf->next = g.surfaceHead;
			g.surfaceHead = sf;
			return;
		}
	}
	if (strncasecmp(line, "sphere:", 7)==0) {
		Sphere *s = (Sphere *)malloc(sizeof(Sphere));
		getVector(fpi, s->center);
		getScalar(fpi, &s->radius);
		//scale the radius if necessary.
		transformScaling(&s->radius);
		s->r2 = s->radius * s->radius;
		doTransform(s->center);
		s->clip=NULL;
		if (g.nclips) {
			//put all clips into place
			int i;
			for (i=0; i<g.nclips; ++i) {
				Clip *c = (Clip *)malloc(sizeof(Clip));
				c->next = s->clip;
				s->clip = c;
				VecCopy(g.cliplist[i].center, s->clip->center);
				VecCopy(g.cliplist[i].norm, s->clip->norm);
				s->clip->radius = g.cliplist[i].radius;
			}
		}
		//s->surface = surfaceHead;
		s->next = g.sphereHead;
		g.sphereHead = s;
		Object *o = (Object *)malloc(sizeof(Object));
		initObject(o, OBJECT_TYPE_SPHERE, g.sphereHead);
		o->intersectFunc = &raySphereIntersect;
		o->normalFunc = &sphereNormal;
		bbSphere(s, o);
		o->next = g.objectHead;
		g.objectHead = o;
		return;
	}
	if (strncasecmp(line, "sphereblob:", 11)==0) {
		int n;
		Vec c,b;
		getVector(fpi, c);
		getVector(fpi, b);
		getInteger(fpi, &n);
		//void buildSphereblob(Vec c, Vec b, int n);
		buildSphereblob(c,b,n);
		return;
	}
}

void scene_t(char *line, FILE *fpi)
{
	if (strncasecmp(line, "tri:", 4)==0) {
		Vec tv[3];
		getVector(fpi, tv[0]);
		getVector(fpi, tv[1]);
		getVector(fpi, tv[2]);
		makeTriangle(tv);
		return;
	}
	if (strncasecmp(line, "tri-texcoord:", 13)==0) {
		int i;
		for (i=0; i<3; i++) {
			getScalar(fpi, &g.triHead->texcoord[i][0]);
			getScalar(fpi, &g.triHead->texcoord[i][1]);
		}
		return;
	}
	#ifdef ALLOW_PATCHES
	if (strncasecmp(line, "tri-patch:", 10)==0) {
		int i;
		for (i=0; i<3; i++) {
			getScalar(fpi, &g.triHead->pnorm[i][0]);
			getScalar(fpi, &g.triHead->pnorm[i][1]);
			getScalar(fpi, &g.triHead->pnorm[i][2]);
			VecNormalize(g.triHead->pnorm[i]);
		}
		g.triHead->patch = 1;
		return;
	}
	#endif //ALLOW_PATCHES
	if (strncasecmp(line, "trans-rotate:", 13)==0) {
		//#define degreesToRadians(d)  (((Flt)(d))*PI/(Flt)180.0)
		Vec v;
		Transform *t = (Transform *)malloc(sizeof(Transform));
		getVector(fpi, v);
		t->rotate[0] = degreesToRadians(v[0]);
		t->rotate[1] = degreesToRadians(v[1]);
		t->rotate[2] = degreesToRadians(v[2]);
		identity(t->mat);
		//Move rotation into our new matrix
		rotate(t);
		concatenateToMatrix(t);
		return;
	}
	if (strncasecmp(line, "trans-scale:", 12)==0) {
		Transform *t = (Transform *)malloc(sizeof(Transform));
		getVector(fpi, t->scale);
		scale(t);
		concatenateToMatrix(t);
		return;
	}
	if (strncasecmp(line, "trans-translate:", 16)==0) {
		//Log("trans-translate:\n");
		Transform *t = (Transform *)malloc(sizeof(Transform));
		getVector(fpi, t->translate);
		//Log("%lf %lf %lf\n",t->translate[0],t->translate[1],t->translate[2]);
		translate(t);
		concatenateToMatrix(t);
		return;
	}
	if (strncasecmp(line, "trans-pop:", 10)==0) {
		//Parameter is optional. blank=1
		int npops;
		int spot = ftell(fpi);
		getInteger(fpi, &npops);
		if (npops==0) {
			npops=1;
			fseek(fpi, spot, SEEK_SET);
		}
		Transform *t;
		while (npops > 0) {
			//Even if user enters large number of pops,
			//the stack integrity is safe.
			if (g.transformHead != NULL) {
				//Pop a transform off the stack...
				t = g.transformHead;
				g.transformHead = t->next;
				free(t);
			}
			--npops;
		}
		return;
	}
}

void scene_u(char *line, FILE *fpi)
{
	if (strncasecmp(line, "up:", 3)==0) {
		getVector(fpi, g.studio.up);
		return;
	}
}

int readScene(void) {
	FILE *fpi;
	if (files.getNSceneFiles() <= 0)
		return 0;
	initScene();
	char ts[256];
	//sprintf(ts, "./scenes/%s", getCurrentSceneName());
	//sprintf(ts, "%s%s", scenePath, getCurrentSceneName());
	sprintf(ts,"%s%s", files.getFilePath(),
			           files.getCurrentSceneName());
	fpi = fopen(ts, "r");
	if (!fpi)
		return 0;
	//------------
	fileloop(fpi);
	//------------
	fclose(fpi);
	//special case of animation
	if (g.anim.running && g.anim.nframes > 1) {
		//this adds animated objects to the scene.
		if (g.anim.frameno % 10 == 0) {
			printf("%i ", g.anim.frameno); fflush(stdout);
		}
		switch (g.anim.number) {
			case ANIM_CLOTH_FALLS_ON_SPHERE: anim_loop_a(); break;
			case ANIM_CLOTH_DRAGGED_ACROSS_SCENE: anim_loop_b(); break;
			case ANIM_TABLECLOTH: anim_loop_c(); break;
			case ANIM_SPHERE: anim_loop_d(); break;
			case ANIM_CUBE: anim_loop_e(); break;
		}
	}
	if (g.cubemap.build) {
		adjustCubeAngle();
	}
	return 1;
}

void initScene(void)
{
	VecZero(g.studio.ambient);
	g.studio.shoot = SHOOT_STRAIGHT;
	g.studio.sky.type  = SKY_TYPE_NONE;
	g.studio.maxDepth  = 4;
	g.studio.minWeight = 0.1;
	g.studio.hazeOn    = 0;
	g.ncpatterns       = 0;
	g.nclips           = 0;
	g.studio.jitter            = 0;
	g.studio.aperture.nsamples = 0;
	MakeVector(1,1,1,g.studio.ambient);
	MakeVector(0,0,100,g.studio.from);
	MakeVector(0,0,0,g.studio.at);
	MakeVector(0,1,0,g.studio.up);
	g.studio.ambientOcclusionN = 0;
	g.studio.ambientOcclusionFunc = 2;
	g.studio.angle = 45.0;
	g.cubemap.active    = 0;
	g.nphotons          = 0;
	g.photonSeeThem     = 0;
	g.photonShadows     = 0;
	g.anim.nframes    = 0;
	g.anim.frameStill = 0;
	g.anim.frameStart = 0;
	g.anim.frameStop  = 0;
	g.anim.frameMod  = 1;
	g.anim.saveFiles  = 0;
	g.hazeHead = NULL;
	//Build a default surface
	//This avoids a crash if user forgets to make a surface.
	Surface *s = (Surface *)malloc(sizeof(Surface));
	s->tm=NULL;
	MakeVector(1,0,0,s->diffuse);
	s->next = g.surfaceHead;
	g.surfaceHead = s;
}

//list of key words
// at: position[3]
// ambient: color[3]
// aperture: radius, aperture.focal_length, nsamples
// angle: view angle
// background: color[3]
// box: center[3], width, height, depth
// checker: scale[3], color[3], color[3]
// clip: center[3], normal[3]
// clip-pop: number of pops
// cylinder: radius, height
// cone: center[3], radius, base, apex
// cubemap: filename
// distribution: 1=on 0=off
// from: position[3]
// heightmap: filename
// haze: color[3], density, limit
// jitter: value
// light: position[3]
// light-center: position[3]
// light-color: color[3]
// light-spec: specular highlight on/off
// light-area: radius, nsamples
// maxdepth: recursion level
// minweight: minimum weight
// montecarlo: nsamples
// montecarloA: nsamples, value
// montecarloB: nsamples, value
// model: filename
// nframes: 1
// ring: position[3], normal[3], min-radius, max-radius
// row-start: row
// row-end: row
// sky: none
// sky-cube: none
// sky-perlin: color[3], color[3], scale[3], horizon
// surf: diffuse-color[3]
// surf-perlin: color[3], color[3], scale[3]
// surf-diffuse: color[3]
// surf-specular: color[3]
// surf-spherical:
// surf-transmit: color-value[3]
// surf-highlight: 0=no, 1=yes
// surf-emit: color[3]
// surf-photons: 1
// surf-ior: entering index-of-refraction, leaving index-of-refraction
// surf-spot: none
// surf-pattern: pattern number
// surf-texmap: filename
// surf-translucent: value, nsamples
// surf-insideout: none
// surf-copy: diffuse-color[3] optional
// sphere: position[3], radius
// sphereblob: center[3], bounding-box[3], nspheres
// tri: vertex[3], vertex[3], vertex[3]
// trans-scale: scale[3]
// trans-translate: vector[3]
// trans-rotate: degrees[3] one at a time
// trans-pop: number of pops
// up: vector[3]


char *leftClip(char *str) {
	while (isspace(*str))
		str++;
	return str;
}

char *rightClip(char *str) {
	char *s = str;
	while (*s != '\0' && *s != 10 && *s != 13)
		s++;
	*s = '\0';
	return str;
}

void fileloop(FILE *fpi) {
	char token[MAXLINE+1], *line;
	//while (fgets(x, MAXLINE, fpi) != NULL) {
	while (!feof(fpi)) {
		int ret = fscanf(fpi, "%s", token);
		if (feof(fpi)) break;
		if (ret) {
			line = token;
			//printf("%s\n",token); fflush(stdout);
			if (token[0]=='#' || (token[0]=='/'&&token[1]=='/')) {
				int c;
				//printf("%s\n",token); fflush(stdout);
				//printf("read until end of line...\n"); fflush(stdout);
				do {
					c = fgetc(fpi);
				} while(!(c==10||c==13||c==EOF));
				continue;
			}
			switch(*line) {
				case 'a': scene_a(line, fpi); break;
				case 'b': scene_b(line, fpi); break;
				case 'c': scene_c(line, fpi); break;
				case 'd': scene_d(line, fpi); break;
				case 'e': scene_e(line, fpi); break;
				case 'f': scene_f(line, fpi); break;
				case 'h': scene_h(line, fpi); break;
				case 'i': scene_i(line, fpi); break;
				case 'j': scene_j(line, fpi); break;
				case 'l': scene_l(line, fpi); break;
				case 'm': scene_m(line, fpi); break;
				case 'n': scene_n(line, fpi); break;
				//case 'o': scene_o(line, fpi); break;
				case 'p': scene_p(line, fpi); break;
				case 'r': scene_r(line, fpi); break;
				case 's': scene_s(line, fpi); break;
				case 't': scene_t(line, fpi); break;
				case 'u': scene_u(line, fpi); break;
			}
		}
	}
	cleanupVariables();
}




















