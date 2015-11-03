#ifndef _ANIMATE_H_
#define _ANIMATE_H_

#define ANIM_CLOTH_FALLS_ON_SPHERE      0
#define ANIM_CLOTH_DRAGGED_ACROSS_SCENE 1
#define ANIM_TABLECLOTH                 2
#define ANIM_SPHERE                     3
#define ANIM_CUBE                       4

typedef struct t_Mass {
	Flt mass, oomass;
	Vec pos, vel, force;
	int grabbed;
	Vec norm;
	int nnorm;
} Mass;

typedef struct t_Spring {
	int mass[2];
	Flt length;
} Spring;

struct Cloth {
	Mass *mass;
	int nmasses;
	Flt step;
	Spring *spring;
	int nsprings;
	int h, w;
	Flt stiffness, stretchiness;
	//
	int clothSmoothing;
	Vec clothCenter;
	Vec clothVelocity;
	int clothWidth;
	int clothHeight;
	Flt clothStep;
	int clothGrabbed[16];
	int clothNgrabbed;
	Flt clothGravity;
	Flt clothDamping;
	int clothSpringSteps;
	char clothTexture[256];
	Flt clothTexTile[2];
};

extern void fileloop(FILE *fpi);
extern void animControl(int num);
extern void checkForSkip();
extern void setupMasses(Vec center);
extern Flt springLen(Spring *s);
extern void setupSprings();
extern void maintain_springs();
extern void anim_loop_a(void);
extern void anim_loop_b(void);
extern void anim_loop_c(void);
extern void anim_loop_d(void);
extern void anim_loop_e(void);

#endif //_ANIMATE_H_

