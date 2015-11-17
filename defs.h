#ifndef _DEFS_H
#define _DEFS_H

//#define USE_GLFW

//cs371 Fall 2013
//program: defs.h
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: framework for ray tracing

typedef double Flt;
typedef Flt Vec[3];
typedef Flt	Matrix[4][4];
typedef Vec	bobPoint;

//some macros to for vectors
#define MakeVector(x, y, z, v) (v)[0] = (x), \
							   (v)[1] = (y), \
							   (v)[2] = (z)

#define VecZero(v) (v)[0] = 0.0; \
				   (v)[1] = 0.0; \
				   (v)[2] = 0.0

#define VecNegate(a) (a)[0] = (-(a)[0]); \
					 (a)[1] = (-(a)[1]); \
					 (a)[2] = (-(a)[2])

#define VecDot(a,b) ((a)[0]*(b)[0]+ \
					 (a)[1]*(b)[1]+ \
					 (a)[2]*(b)[2])

#define VecLen(a) ((a)[0] * (a)[0] + \
				   (a)[1] * (a)[1] + \
				   (a)[2] * (a)[2])

#define VecLenSq(a) sqrt(             \
					(a)[0] * (a)[0] + \
					(a)[1] * (a)[1] + \
					(a)[2] * (a)[2])

#define VecCopy(a,b) (b)[0]=(a)[0]; \
					 (b)[1]=(a)[1]; \
					 (b)[2]=(a)[2]

#define VecCopyNeg(a,b) (b)[0] = -(a)[0]; \
						(b)[1] = -(a)[1]; \
						(b)[2] = -(a)[2]

#define VecAdd(a,b,c) (c)[0] = (a)[0] + (b)[0]; \
					  (c)[1] = (a)[1] + (b)[1]; \
					  (c)[2] = (a)[2] + (b)[2]

#define VecSub(a,b,c) (c)[0] = (a)[0] - (b)[0]; \
					  (c)[1] = (a)[1] - (b)[1]; \
					  (c)[2] = (a)[2] - (b)[2]

#define VecMul(a,b,c) (c)[0] = (a)[0] * (b)[0]; \
					  (c)[1] = (a)[1] * (b)[1]; \
					  (c)[2] = (a)[2] * (b)[2]

#define VecS(A,a,b) (b)[0] = (A) * (a)[0]; \
					(b)[1] = (A) * (a)[1]; \
					(b)[2] = (A) * (a)[2]

#define VecAddS(A,a,b,c) (c)[0] = (A) * (a)[0] + (b)[0]; \
						 (c)[1] = (A) * (a)[1] + (b)[1]; \
						 (c)[2] = (A) * (a)[2] + (b)[2]

#define VecCross(a,b,c) (c)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1]; \
						(c)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2]; \
						(c)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0]

//This macro combines 2 vectors
#define VecComb(A,a,B,b,c) (c)[0] = (A) * (a)[0] + (B) * (b)[0]; \
						   (c)[1] = (A) * (a)[1] + (B) * (b)[1]; \
						   (c)[2] = (A) * (a)[2] + (B) * (b)[2]

//Absolute values
#define VecABS(v) (v)[0] = ((v)[0] < 0.0) ? -(v)[0] : (v)[0]; \
				  (v)[1] = ((v)[1] < 0.0) ? -(v)[1] : (v)[1]; \
				  (v)[2] = ((v)[2] < 0.0) ? -(v)[2] : (v)[2]

#define ClampRGB(rgb) (rgb)[0]=((rgb)[0]>1.0f)?1.0f:(rgb)[0]; \
					  (rgb)[1]=((rgb)[1]>1.0f)?1.0f:(rgb)[1]; \
					  (rgb)[2]=((rgb)[2]>1.0f)?1.0f:(rgb)[2]

//This macro finds a point along a ray: point = origin + (t * dir) 
#define RayPoint(ray,t,point) VecAddS((t),(ray)->d,(ray)->o,point)

//Want inline function in multiple extern files... use macro.
#define nudgeRayForward(ray) (ray)->o[0] += (ray)->d[0] * 0.0001; \
                             (ray)->o[1] += (ray)->d[1] * 0.0001; \
                             (ray)->o[2] += (ray)->d[2] * 0.0001

//some constant values
#define IOR_AIR      1.000293
#define IOR_WATER    1.33
#define IOR_GLASS    1.52
#define IOR_SAPPHIRE 1.77
#define IOR_DIAMOND  2.417
#define PI 3.1415926535897932384626433832795
//other macros
#define ABS(a) (((a)<0)?(-(a)):(a))
#define SGN(a) (((a)<0)?(-1):(1))
#define SGND(a) (((a)<0.0)?(-1.0):(1.0))
#define random(a) rand() % (a)
#define rnd() (Flt)rand()/(Flt)RAND_MAX
#define degreesToRadians(d)  ((d) * PI / 180.0)
#define iswap(a,b) (a)^=(b);(b)^=(a);(a)^=(b)
#define NSLABS (3)

//Structures

typedef struct t_bsphere {
	//bounding sphere
	Vec center;
	Flt radius;
	Flt radiusSqr;
} Bsphere;

typedef struct t_bbox {
	//bounding box
	Flt min[3];
	Flt max[3];
} Bbox;

typedef struct t_ray {
	Vec o; //origin
	Vec d; //direction
} Ray;

typedef struct t_hit {
	Flt t;
	Vec p, norm;
	Vec color;
	Vec nhit; //normal at hit surface
} Hit;

#define TRANSFORM_TRANSLATE 0x00000001
#define TRANSFORM_ROTATE    0x00000010
#define TRANSFORM_SCALE     0x00000100
typedef struct t_transform {
	Vec translate, rotate, scale;
	Flt mat[4][4];
	struct t_transform *next;
} Transform;

#define MAXCLIPS 32
typedef struct t_clip {
	Vec center, norm;
	Vec ucenter, unorm;
	Flt radius;
	struct t_clip *next;
} Clip;

typedef struct t_aperture {
	Flt radius, r5;
	Flt focalLength;
	int nsamples;
} Aperture;

typedef struct t_translucent {
	Flt val;
	int nsamples;
	int YN;
} Translucent;

#define SKY_TYPE_NONE        0
#define SKY_TYPE_BLUE        1
#define SKY_TYPE_PERLIN      2
#define SKY_TYPE_ENVIRONMENT 3

typedef struct t_sky {
	int type;
	Vec color[2];
	Vec scale;
	Flt horizon;
} Sky;


#define SHOOT_STRAIGHT     0x00000000
#define SHOOT_DISTRIBUTE4  0x00000010
#define SHOOT_APERTURE     0x00000100
#define SHOOT_MONTECARLO   0x00001000
#define SHOOT_ADAPTIVE     0x00010000

typedef struct t_studio {
	unsigned int shoot;
	int row_start;
	int row_end;
	int maxDepth;
	Flt minWeight;
	Vec from, at, up;
	Vec ambient;
	Vec background;
	Sky sky;
	Flt angle;
	Aperture aperture;
	int nsamples;
	int jitter;
	int hazeOn;
	int ambientOcclusionN;
	Flt ambientOcclusionDist;
	int ambientOcclusionFunc;
	Flt jval, jval2;
	Flt var;
} Studio;

typedef struct t_anim {
	int running;
	int number;
	int nframes;
	int frameStill;
	int frameStart;
	int frameStop;
	int frameMod;
	int frameno;
	int saveFiles;
	int skip;
	int frameRendered;
	char directory[100];
} Anim;

typedef struct t_texmap {
	int xres, yres;
	//imageMagic convert will be used to handle png, gif, jpg, ppm.
	//A name is stored here only if the texture file was not a PPM.
	//This ppm file will be deleted.
	//char *ppmName;
	unsigned char *data;
} Texmap;

typedef struct t_surface {
	int type;
	int insideout;
	int pattern;
	int checker_idx;
	Vec diffuse, specular, transmit, highlight, emit;
	int specularYN, transmitYN, highlightYN, emitYN, photonsYN;
	int sub;
	Translucent translucent;
	Vec perlin_color[2];
	Vec perlin_scale;
	Flt spot;
	Flt iorOutside;
	Flt iorInside;
	Matrix mat, invmat;
	Texmap *tm;
	int spherical;
	int cylindrical;
	struct t_surface *next;
} Surface;

//function pointer tutorial
//http://www.newty.de/fpt/intro.html

#define OBJECT_TYPE_SPHERE   1
#define OBJECT_TYPE_TRI      2
#define OBJECT_TYPE_RING     3
#define OBJECT_TYPE_CYLINDER 4
#define OBJECT_TYPE_CONE     5
typedef struct t_object {
	int type;
	Bbox bb;
	void *obj;
	Surface *surface;
	//function pointers
	int (*intersectFunc)(struct t_object *, Ray *, Hit *);
	void (*normalFunc)(struct t_object *, Hit *, Vec norm);
	//pointer to a linked list of objects to be tested
	struct t_object *head;
	//pointer to the adjacent objects in a list
	struct t_object *next;
	struct t_object *prev;
} Object;

typedef struct t_bvh {
	//Bounding volume hierarchy
	Bbox bb;
	Object *ohead;
	//This points to the next bb objects
	struct t_bvh *left;
	struct t_bvh *right;
} BVH;

typedef struct t_light {
	int specular;
	int area;
	int nsamples;
	int nphotons;
	int falloff;
	Flt falloffVal, oofalloffVal;
	Flt radius;
	Flt photonPower;
	Vec photonDir;
	Vec center;
	Vec color;
	Object *cache_obj;
	struct t_light *next;
} Light;

typedef struct t_photon {
	Vec p;
	Vec norm;
	Vec col;
} Photon;

typedef struct t_sphere {
	Vec center;
	Flt radius, r2;
	Clip *clip;
	struct t_sphere *next;
} Sphere;

#define ALLOW_PATCHES

typedef struct t_tri {
	Vec verts[3];
	Vec norm;
	Flt texcoord[3][2];
	Flt uvw[3];
	#ifdef ALLOW_PATCHES
	int patch;
	Vec pnorm[3];
	#endif //ALLOW_PATCHES
	Clip *clip;
	struct t_tri *next;
} Tri;

typedef struct t_ring {
	Vec center, norm;
	Flt maxradius, minradius;
	Clip *clip;
	struct t_ring *next;
} Ring;

typedef struct t_cylinder {
	Flt radius;
	Flt apex;
	Clip *clip;
	struct t_cylinder *next;
} Cylinder;

typedef struct t_cone {
	Vec center;
	Flt radius;
	Flt base, apex;
	Clip *clip;
	struct t_cone *next;
} Cone;

typedef struct t_temporaryPPM {
	//imageMagic convert will be used to handle png, gif, jpg, ppm.
	//A name is stored here only if the texture file was not a PPM.
	//This ppm file will be deleted.
	char *ppmName;
	struct t_temporaryPPM *next;
} TemporaryPPM;


#define HAZE_TYPE_NORMAL  1
#define HAZE_TYPE_SPHERES 2
#define HAZE_TYPE_BORDERS 3
#define HAZE_TYPE_FOG     4

typedef struct t_haze {
	unsigned int haze_on;
	unsigned int type;
	unsigned int fog;
	Flt linear;
	Vec center;
	Vec reach;
	Vec boundryMax;
	Vec boundryMin;
	Flt density, depth;
	Flt minRadius, maxRadius;
	Flt minRad2, maxRad2;
	Flt ground, minTop, maxTop;
	Vec color;
	unsigned int colorFlag;
	Flt distMin,distMax;
	Vec p[8];
	Vec polyPoint[6][4];
	Vec polyNormal[6];
	char polyP1[6];
	char polyP2[6];
	Flt polyD[6];
	Flt totalDist;
	int media;
	Flt mediaStep;
	Flt mediaDensity;
	int mediaAdaptive;
	Clip *clips;
	struct t_haze *next;
} Haze;


#define MAXCPATTERNS 16
typedef struct t_cpattern {
	Vec scale;
	Vec color0;
	Vec color1;
	Vec color2;
	Flt groutWidth;
	int grout;
} Cpattern;

typedef struct t_mouse {
	int x,y;
	int lastx,lasty;
	int lbuttondown;
} Mouse;

typedef struct t_myscreen {
	int xres, yres;
	int xres3;
	Flt fyres, fxres;
	Flt xcenter, ycenter;
} myScreen;

typedef struct t_mats {
	Matrix worldSpace;
	Matrix cameraSpace;
	Matrix playerSpace;
} Mats;

typedef struct t_frustum {
	Flt frontPlaneDistance;
	Vec frontPlaneNormal;
	Vec frontPlanePoint;
	Flt farPlaneDistance;
	Vec farPlanePoint;
	Vec planes[6];
} Frustum;

typedef struct t_cubemap {
	int build;
	int cnt;
	int sideres;
	Vec from;
	int active;
	void *data;
} Cubemap;

enum {
	MODE_NONE=0,
	MODE_DRAW,
	MODE_HELP,
	MODE_FILES,
	MODE_WINDOW_SIZE,
	MODE_ANTIALIAS,
	MODE_INTERVAL
};

typedef struct t_global {
	int mode, prevmode;
	int version;
	int done;
	int tracing;
	int traceResolution;
	int readyToPrint;
	int updateScreen;
	int filesRowCol[2];
	int renderInterval;
	//int timeEstimation;
	double traceTime;
	//mouse rectangle dragged
	int rspot[2];
	int r1[4];
	int r2[4];
	int rFlag;
	float var[26];
	//clicked on image
	int cFlag;
	Vec cat;
	//
	Studio studio;
	myScreen screen;
	Anim anim;
	//linked lists
	Transform *transformHead;
	Light *lightHead;
	Surface *surfaceHead;
	Object *objectHead;
	Sphere *sphereHead;
	Tri *triHead;
	Ring *ringHead;
	Cylinder *cylinderHead;
	Cone *coneHead;
	BVH *bvhHead;
	Haze *hazeHead;
	TemporaryPPM *temporaryPPMHead;
	//
	//antialias override
	int aaOn;
	int aaCount;
	Flt aaTolerance;
	//
	Cubemap cubemap;
	Cpattern cpattern[MAXCPATTERNS];
	int ncpatterns;
	Clip cliplist[MAXCLIPS];
	int nclips;
	float *screendata;
	//
	Photon *photonmap;
	int nphotons;
	int photonSeeThem;
	int photonShadows;
} Global;

#endif

