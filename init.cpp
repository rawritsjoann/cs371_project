//cs371 Fall 2013
//program: init.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: framework for ray tracing
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h> 
#include<GL/glx.h>
#include "defs.h"
#include "extern.h"
#include "files.h"
#include "fonts.h"
#include "log.h"

void setWinSize(int x, int y)
{
	//Configure window value mask bits
	//#define CWX			(1<<0)
	//#define CWY			(1<<1)
	//#define CWWidth		(1<<2)
	//#define CWHeight		(1<<3)
	//#define CWBorderWidth	(1<<4)
	//#define CWSibling		(1<<5)
	//#define CWStackMode	(1<<6)
	//Values
	//typedef struct {
	//	int x, y;
	//	int width, height;
	//	int border_width;
	//	Window sibling;
	//	int stack_mode;
	//} XWindowChanges;
	//Log("void setWinSize(%i, %i)...\n");
	unsigned int value_mask=0;
	//value_mask |= (1<<2);
	//value_mask |= (1<<3);
	value_mask |= CWWidth;
	value_mask |= CWHeight;
	//
	XWindowChanges xwc;
	xwc.width = x;
	xwc.height = y;
	XConfigureWindow(dpy, win, value_mask, &xwc);
}

void setTitle(double t)
{
	//Set the window title bar.
	char ts[128];
	if (g.mode == MODE_INTERVAL) {
		sprintf(ts, "Render interval: press 1 through 8, or ~");
	} else {
		sprintf(ts, "%ix%i <%s>  %3.2lf <F1>Help  P=%i I=%i",
							g.screen.xres, g.screen.yres,
							files.getCurrentSceneName(), t,
							g.traceResolution, g.renderInterval);
	}
	XMapWindow(dpy, win);
	XStoreName(dpy, win, ts);
}

int checkOpenglVersion()
{
	//Get the OpenGL version info via glxinfo.
	//This is necessary because Mesa differs from Nvidia drivers.
	//In this program, it can be seen when drawing lines with GL_LINES.
	//Position of lines differ by 1-pixel in the y-direction.
	//---------------------------------------------------------------------
	//A student could turn this into a research project. Study this to find
	//out why it is happening, which drivers are considered correct, and
	//find various ways to correct the problem.
	if (system("glxinfo | grep \"OpenGL version string:\" > v.txt")) {
		printf("Error running system command.\n"); fflush(stdout);
		return 1;
	}
	FILE *fpi = fopen("v.txt","r");
	char ts[256];
	if (fgets(ts, 256, fpi)) { }
	fclose(fpi);
	if (strstr(ts, "Mesa")) return 1;
	if (strstr(ts, "NVIDIA")) return 2;
	return 1;
}

void allocateScreendata(void)
{
	//Screen pixels are stored in screendata.
	if (g.screendata)
		free(g.screendata);
	int size = g.screen.xres*g.screen.yres*3*sizeof(float);
	g.screendata = (float *)malloc(size);
	memset(g.screendata, 0, size);
}

void setupScreenRes(const int w, const int h)
{
	g.screen.xres = w;
	g.screen.yres = h;
	g.screen.fxres = (Flt)g.screen.xres;
	g.screen.fyres = (Flt)g.screen.yres;
	g.screen.xres3 = g.screen.xres * 3;
}

void initXwindow(void)
{
	//double buffer was needed for Linux 14.10
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	//GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
	XSetWindowAttributes swa;
	setupScreenRes(600, 600);
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		printf("\n\tcannot connect to X server\n\n");
		exit(0);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		printf("\n\tno appropriate visual found\n\n");
		exit(0);
	}
	//else {
	//	// %p creates hexadecimal output like in glxinfo
	//	printf("\n\tvisual %p selected\n", (void *)vi->visualid);
	//}
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | StructureNotifyMask |
					PointerMotionMask | ButtonPressMask |
					ButtonReleaseMask | KeyPressMask |
					KeyReleaseMask;
	win = XCreateWindow(dpy, root, 0, 0, g.screen.xres, g.screen.yres, 0,
							vi->depth, InputOutput, vi->visual,
							CWColormap | CWEventMask, &swa);
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "RayTrace");
	GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
	//glEnable(GL_DEPTH_TEST); 
	//Display *dpy = glXGetCurrentDisplay();
	//GLXDrawable drawable = glXGetCurrentDrawable();
	//const int interval = 1;
	//if (drawable) {
	//	glXSwapIntervalEXT(dpy, drawable, interval);
	//}
}

void reshapeWindow(int width, int height)
{
	//window has been resized.
	setupScreenRes(width, height);
	setWinSize(g.screen.xres, g.screen.yres);
	allocateScreendata();
	//
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glOrtho(0, g.screen.xres, 0, g.screen.yres, -1, 1);
	setTitle(0.0);
	//printf("xxx\n");
}

void initOpengl(void)
{
	Log("initOpengl()...\n");
	//OpenGL initialization
	glViewport(0, 0, g.screen.xres, g.screen.yres);
	//init matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//This sets 2D mode (no perspective)
	glOrtho(0, g.screen.xres, 0, g.screen.yres, -1, 1);
	//Clear the screen
	//glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
	glClearColor(0.7, 0.6, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	//Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void init(void)
{
	Log("init()...\n");
	g.mode = MODE_NONE;
	g.done=0;
	g.tracing=0;
	g.readyToPrint=0;
	g.updateScreen=1;
	g.screendata=NULL;
	g.cubemap.active=0;
	g.traceResolution=4;
	g.photonmap=NULL;
	g.rFlag=0;
	g.cFlag=0;
	g.renderInterval=4;
	//int i=0;
	//while (i<26)
	//	g.var[i++]=0;
	memset(g.var, 0, sizeof(int)*26);
	g.anim.running=0;
	setTitle(0);
	//
	g.studio.maxDepth=4;
	g.studio.minWeight=0.1;
	//linked lists
	g.transformHead=NULL;
	g.lightHead=NULL;
	g.surfaceHead=NULL;
	g.objectHead=NULL;
	g.sphereHead=NULL;
	g.triHead=NULL;
	g.ringHead=NULL;
	g.cylinderHead=NULL;
	g.coneHead=NULL;
	g.bvhHead=NULL;
	g.hazeHead=NULL;
	g.temporaryPPMHead=NULL;
	//
	files.getFiles();
	srand((unsigned int)time(NULL));
	//
	g.version = checkOpenglVersion();
	allocateScreendata();
}

void cleanupLists(void)
{
	//printf("cleanup_lists()...\n"); fflush(stdout);
	//Free memory used by structures.
	//This is normally called right before a scene is read and traced.
	struct t_clip *tc;
	Clip *c;
	//
	struct t_transform *tm;
	Transform *m  = g.transformHead;
	while(m) {
		tm = m->next;
		free(m);
		m = tm;
	}
	g.transformHead = NULL;
	//
	struct t_light *tl;
	Light *l = g.lightHead;
	while(l) {
		tl = l->next;
		free(l);
		l = tl;
	}
	g.lightHead = NULL;
	//
	struct t_surface *tf;
	Surface *f = g.surfaceHead;
	while(f) {
		if (f->tm) {
			//texture map must be freed.
			free(f->tm);
			f->tm = NULL;
		}
		tf = f->next;
		free(f);
		f = tf;
	}
	g.surfaceHead = NULL;
	//
	struct t_object *to;
	Object *o = g.objectHead;
	while(o) {
		to = o->next;
		free(o);
		o = to;
	}
	g.objectHead = NULL;
	//
	struct t_sphere *ts;
	Sphere *s = g.sphereHead;
	while(s) {
		//sphere might have clips...
		if (s->clip) {
			c = s->clip;
			while(c) {
				tc = c->next;
				free(c);
				c = tc;
			}
		}
		ts = s->next;
		free(s);
		s = ts;
	}
	g.sphereHead = NULL;
	//
	struct t_tri *tt;
	Tri *t = g.triHead;
	while(t) {
		//tri might have clips...
		if (t->clip) {
			c = t->clip;
			while(c) {
				tc = c->next;
				free(c);
				c = tc;
			}
		}
		tt = t->next;
		free(t);
		t = tt;
	}
	g.triHead = NULL;
	//
	struct t_ring *tr;
	Ring *r = g.ringHead;
	while(r) {
		//ring might have clips...
		if (r->clip) {
			c = r->clip;
			while(c) {
				tc = c->next;
				free(c);
				c = tc;
			}
		}
		tr = r->next;
		free(r);
		r = tr;
	}
	g.ringHead = NULL;
	//
	struct t_cylinder *tcy;
	Cylinder *cy = g.cylinderHead;
	while(cy) {
		if (cy->clip) {
			c = cy->clip;
			while(c) {
				tc = c->next;
				free(c);
				c = tc;
			}
		}
		tcy = cy->next;
		free(cy);
		cy = tcy;
	}
	g.cylinderHead = NULL;
	//
	struct t_cone *tcn;
	Cone *cn = g.coneHead;
	while(cn) {
		if (cn->clip) {
			c = cn->clip;
			while(c) {
				tc = c->next;
				free(c);
				c = tc;
			}
		}
		tcn = cn->next;
		free(cn);
		cn = tcn;
	}
	g.coneHead = NULL;
	//
	struct t_haze *thz;
	Haze *hz = g.hazeHead;
	while(hz) {
		thz = hz->next;
		free(hz);
		hz = thz;
	}
	g.hazeHead = NULL;
	//
	void cleanupBvh(BVH *bvh);
	cleanupBvh(g.bvhHead);
	g.bvhHead=NULL;
}

void cleanupBvh(BVH *bvh)
{
	if (bvh==NULL) return;
	//
	cleanupBvh(bvh->left);
	cleanupBvh(bvh->right);
	//
	Object *o = bvh->ohead;
	while(o) {
		Object *to = o->next;
		free(o);
		o = to;
	}
	free(bvh);
}



void cleanupTemporaryPPMs(void)
{
	struct t_temporaryPPM *tppm;
	TemporaryPPM *hppm = g.temporaryPPMHead;
	while(hppm) {
		tppm = hppm->next;
		if (hppm->ppmName != NULL) {
			remove(hppm->ppmName); //delete the file
			free(hppm->ppmName);   //free memory
		}
		free(hppm);
		hppm = tppm;
	}
	g.temporaryPPMHead = NULL;
}

void cleanup(void)
{
	//As program ends.
	cleanupLists();
	cleanup_fonts();
	cleanupTemporaryPPMs();
	free(g.screendata);
}

