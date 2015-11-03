#ifndef _EXTERN_H
#define _EXTERN_H

#include <X11/Xlib.h>

extern Display *dpy;
extern Window win;
extern Global g;
//function prototypes...
extern void initXwindow(void);
extern void checkResize(XEvent *e);
extern void checkKeys(XEvent *e);
extern void checkMouse(XEvent *e);
extern void init(void);
extern void cleanup(void);
extern void currentWinsizeIncrement(const int dir);
extern void getCurrentWinsize(int *x, int *y);
extern void initOpengl(void);
extern void allocateScreendata(void);
extern void setupScreenRes(const int w, const int h);
extern void initOpengl(void);
extern int readScene(void);
extern void saveImage(float *screendata, int xres, int yres, char *filename);
extern void cleanupLists(void);
extern int raySphereIntersect(Object *obj, Ray *ray, Hit *hit);
extern int rayCylinderIntersect(Object *obj, Ray *ray, Hit *hit);
extern int rayConeIntersect(Object *obj, Ray *ray, Hit *hit);
extern int rayPlaneIntersect(Vec ray_from, Vec ray_dir, Vec plane_center, Vec plane_norm, Hit *hit);
extern int rayRingIntersect(Object *obj, Ray *ray, Hit *hit);
extern int PointInTriangle(Vec A, Vec B, Vec C, Vec P);
extern int rayTriIntersect(Object *obj, Ray *r, Hit *h);
extern void sphereNormal(Object *obj, Hit *hit, Vec norm);
extern void ringNormal(Object *obj, Hit *hit, Vec norm);
extern void triNormal(Object *obj, Hit *hit, Vec norm);
extern void cylinderNormal(Object *obj, Hit *hit, Vec norm);
extern void coneNormal(Object *obj, Hit *hit, Vec norm);
extern void checkkey(int k1, int k2);
extern void render(void);
extern void renderViewport(const int y, const int w, const int h);
extern void getFiles(void);
extern void replaceFileExtension(char *file, const char *ext);
extern void setTitle(double t);
extern void setPixel(int x, int y, Vec rgb);
extern void resolveTexmap(Object *o, Hit *hit);
//
extern float noise2(float vec[2]);
extern float noise3(float vec[3]);
extern void checkerPattern(Vec P, Surface *sf, Vec rgb);
extern float turbulence(float vec[3], int power);
extern void backgroundHit(Ray *ray, Vec rgb);
extern void perlin_texture_2D(Vec p, Surface *sf, Vec rgb);
extern void perlin_texture_3D(Vec p, Surface *sf, Vec rgb);
extern void buildHierarchy(void);
extern int checkSlabs(Bbox *b, Ray *r);
extern void adjustCubeAngle(void);
extern void buildCube(void);
extern void buildCubemapTexture(char *filename);
extern void translate(Transform *t);
extern void rotate(Transform *t);
extern void scale(Transform *t);
extern void castRays(const int dim);
extern void floatdataToChar(float *cubeBits, unsigned char *uptr,
												int xres, int yres);
//
extern void photonShadowBvh(BVH *bvh, Ray *ray, Flt disttolight, int *shad);
extern int photonTraceShadow(Ray *ray, Flt disttolight);
extern void getPhotonContribution(Hit *closehit, Vec nhit, Vec plight);
extern void buildPhotonMap();
//
extern void subsurfaceBvh(BVH *bvh, Ray *ray, Vec nhit, Vec col);
extern void subsurface2(Hit *hit, Vec col);
extern Object *sub_hierarchy(BVH *bvh, Ray *ray, Hit *closehit, Object *closeObj);
//
extern void reshapeWindow(int width, int height);

#endif

