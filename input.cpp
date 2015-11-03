//cs371 Fall 2013
//program: input.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: framework for ray tracing
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <GL/glx.h>
#include "log.h"
#include "defs.h"
#include "extern.h"
#include "ppm.h"
#include "fonts.h"
#include "files.h"
#include "animate.h"
extern void animControl(int num);

void saveSnapshot();
void traceRectangle();
char *hms(char *str, double dsec);
extern double timediff(struct timespec *start, struct timespec *end);
static int lbutton=0;
static int rbutton=0;


void checkKeys(XEvent *e)
{
	static int shift=0;
	static int ctrl=0;
	static int alt=0;
	if (e->type != KeyPress && e->type != KeyRelease)
		return;
	int key = XLookupKeysym(&e->xkey, 0);
	//printf("check_keys()...  %i\n", key);
	if (e->type == KeyPress) {
		if (key == XK_Shift_L || key == XK_Shift_R) {
			shift=1;
			return;
		}
		if (key == XK_Control_L || key == XK_Control_R) {
			ctrl=1;
			return;
		}
		if (key == XK_Alt_L || key == XK_Alt_R) {
			alt=1;
			return;
		}
	} else if (e->type == KeyRelease) {
		//printf("key released...\n"); fflush(stdout);
		//shift, ctrl, or alt key released?
		if (key == XK_Shift_L || key == XK_Shift_R)
			shift=0;
		if (key == XK_Control_L || key == XK_Control_R) {
			ctrl=0;
			//printf("ctrl set off.\n"); fflush(stdout);
		}
		if (key == XK_Alt_L || key == XK_Alt_R)
			alt=0;
		//don't process any other keys on a release
		return;
	} else {
		//if (e->type != KeyPress) {
		//Log("not a key press, returning.\n");
		return;
	}
	if (alt){}
	//-------------------------------------------------------------------------
	if (key == XK_a) {
		animControl(ANIM_CLOTH_FALLS_ON_SPHERE);
		return;
	}
	//-------------------------------------------------------------------------
	if (key == XK_b) {
		animControl(ANIM_CLOTH_DRAGGED_ACROSS_SCENE);
		return;
	}
	//-------------------------------------------------------------------------
	if (key == XK_c) {
		//checkerboard table-cloth falling on a table.
		animControl(ANIM_TABLECLOTH);
		return;
	}
	//-------------------------------------------------------------------------
	if (key == XK_d) {
		//sphere on checkerboard.
		animControl(ANIM_SPHERE);
		return;
	}
	//-------------------------------------------------------------------------
	if (key == XK_e) {
		//sphere on checkerboard.
		animControl(ANIM_CUBE);
		return;
	}
	//-------------------------------------------------------------------------
	if (key == XK_i) {
		//I key sets rendering interval.
		if (g.mode != MODE_INTERVAL) {
			g.prevmode = g.mode;
			g.mode = MODE_INTERVAL;
		} else {
			g.mode = g.prevmode;
		}
		setTitle(0.0);
		return;
	}

	//-------------------------------------------------------------------------
	if (key == XK_t) {
		//estimate time to trace.
		char str[64];
		int dim = g.traceResolution;
		double sec = g.traceTime;
		printf("total time: %3.2lf\n", g.traceTime);
		double totalPixels = (double)(g.screen.xres*g.screen.xres);
		double dimPixels = totalPixels / (double)(dim*dim);
		double perPixel = sec / dimPixels;
		printf("time estimates:\n");
		printf(" 1x 1: %s\n", hms(str, totalPixels * perPixel));
		printf(" 2x 2: %s\n", hms(str, (totalPixels/(2*2)) * perPixel));
		printf(" 4x 4: %s\n", hms(str, (totalPixels/(4*4)) * perPixel));
		printf(" 8x 8: %s\n", hms(str, (totalPixels/(8*8)) * perPixel));
		printf("16x16: %s\n", hms(str, (totalPixels/(16*16)) * perPixel));
		printf("32x32: %s\n", hms(str, (totalPixels/(32*32)) * perPixel));
		return;
	}
	//-------------------------------------------------------------------------
	if (key == XK_F1) {
		g.mode = MODE_HELP;
		//glClear(GL_COLOR_BUFFER_BIT);
		return;
	}
	if (key == XK_F2) {
		//printf("g.mode = MODE_FILES;\n");
		g.mode = MODE_FILES;
		//glClear(GL_COLOR_BUFFER_BIT);
		files.getFiles();
		return;
	}
	if (key == XK_F3) {
		if (files.getNSceneFiles() > 0) {
			if (!files.currentIsDir()) {
				char ts[128];
				//sprintf(ts,"gedit scenes/%s &",get_current_scene_name());
				if (g.version==1)
					sprintf(ts,"gedit %s%s &", files.getFilePath(),
					                           files.getCurrentSceneName());
				if (g.version==2)
					sprintf(ts,"pluma %s%s &", files.getFilePath(),
					                           files.getCurrentSceneName());
				if (system(ts)) {}
			}
		}
		return;
	}
	if (key == XK_F4) {
		//F4 toggles the antialias selection mode.
		//Override antialiasing with montecarlo.
		if (g.mode != MODE_ANTIALIAS) {
			g.prevmode = g.mode;
			printf("savemode = %i\n", g.mode);
			g.mode = MODE_ANTIALIAS;
			int x = 10, w = 100;
			int y = 110, h = 100;
			y = g.screen.yres - y;
			glColor3ub(40,40,40);
			glBegin(GL_QUADS);
				glVertex2i(x, y);
				glVertex2i(x+w,y);
				glVertex2i(x+w, y+h);
				glVertex2i(x,y+h);
			glEnd();
			x += 4, w -= 8;
			y += 4, h -= 8;
			glColor3ub(240,240,40);
			glBegin(GL_QUADS);
				glVertex2i(x, y);
				glVertex2i(x+w,y);
				glVertex2i(x+w, y+h);
				glVertex2i(x,y+h);
			glEnd();
			Rect r;
			r.left = x+10;
			r.bot = y+h-12;
			r.center = 0;
			ggprint8b(&r, 16, 0x00000000, "Antialias");
			ggprint8b(&r, 12, 0x00000066, "1 - 1");
			ggprint8b(&r, 12, 0x00000066, "2 - 5");
			ggprint8b(&r, 12, 0x00000066, "3 - 10");
			ggprint8b(&r, 12, 0x00000066, "4 - 80");
			ggprint8b(&r, 12, 0x00000066, "5 - 200");
			g.updateScreen=1;
		} else {
			g.mode = g.prevmode;
			g.aaOn = 0;
			glRasterPos2i(0,0);
			glDrawPixels(g.screen.xres, g.screen.yres,
							GL_RGB, GL_FLOAT, (const GLvoid *)g.screendata);
			g.updateScreen=1;
		}
		return;
	}
	if (key == XK_F6) {
		saveSnapshot();
		return;
	}
	if (key == XK_F7 && ctrl) {
		if (g.screen.xres != g.screen.yres) {
			printf("Screen must be square to trace a cubemap.\n");
			return;
		}
		g.cubemap.cnt=0;
		g.mode = MODE_NONE;
		g.cubemap.build=1;
		//Trace all 6 views
		while(g.cubemap.cnt < 6) {
			castRays(g.traceResolution);
			buildCube();
		}
		g.cubemap.build=0;
		return;
	}
	if (key == XK_F9) {
		//select screen size
		g.mode = MODE_WINDOW_SIZE;
		return;
	}
	if (key == XK_Down) {
		if (!(shift+ctrl)) {
			//shift and ctrl are not used to navigate lists
			if (g.mode == MODE_FILES) {
				files.currentSceneIncrement(1);
				return;
			}
			if (g.mode == MODE_WINDOW_SIZE) {
				currentWinsizeIncrement(1);
				return;
			}
		}
		if (shift) {
			//http://stackoverflow.com/questions/3806872/window-position-in-xlib
			XWindowAttributes xwa;
			int x, y;
			Window child;
			Window root = DefaultRootWindow(dpy);
			XTranslateCoordinates(dpy, win, root, 0, 0, &x, &y, &child );
			XGetWindowAttributes(dpy, win, &xwa);
			//printf("window position: %i %i\n", x-xwa.x, y-xwa.y);
			XMoveWindow(dpy, win, x-xwa.x, y-xwa.y-28+8);		
		} else {
			g.screen.yres += (1 + shift*9 + ctrl*19);
			if (g.screen.yres > 2000)
				g.screen.yres = 2000;
			reshapeWindow(g.screen.xres, g.screen.yres);
		}
		return;
	}
	if (key == XK_Up) {
		if (!(shift+ctrl)) {
			//shift and ctrl are not used to navigate lists
			if (g.mode == MODE_FILES) {
				files.currentSceneIncrement(-1);
				return;
			}
			if (g.mode == MODE_WINDOW_SIZE) {
				currentWinsizeIncrement(-1);
				return;
			}
		}
		if (shift) {
			//http://stackoverflow.com/questions/3806872/window-position-in-xlib
			XWindowAttributes xwa;
			int x, y;
			Window child;
			Window root = DefaultRootWindow(dpy);
			XTranslateCoordinates(dpy, win, root, 0, 0, &x, &y, &child );
			XGetWindowAttributes(dpy, win, &xwa);
			//printf("window position: %i %i\n", x-xwa.x, y-xwa.y);
			XMoveWindow(dpy, win, x-xwa.x, y-xwa.y-28-8);		
		} else {
			g.screen.yres -= (1 + shift*9 + ctrl*19);
			if (g.screen.yres < 2)
				g.screen.yres = 2;
			reshapeWindow(g.screen.xres, g.screen.yres);
		}
		return;
	}
	if (key == XK_Left) {
		if (!(shift+ctrl)) {
			//shift and ctrl are not used to navigate lists
			if (g.mode == MODE_FILES) {
				files.currentSceneIncrement(-g.filesRowCol[0]);
				return;
			}
		}
		if (shift) {
			//http://stackoverflow.com/questions/3806872/window-position-in-xlib
			XWindowAttributes xwa;
			int x, y;
			Window child;
			Window root = DefaultRootWindow(dpy);
			XTranslateCoordinates(dpy, win, root, 0, 0, &x, &y, &child );
			XGetWindowAttributes(dpy, win, &xwa);
			//printf("window position: %i %i\n", x-xwa.x, y-xwa.y);
			XMoveWindow(dpy, win, x-xwa.x-8, y-xwa.y-28);		
		} else {
			g.screen.xres -= (1 + shift*9 + ctrl*19);
			if (g.screen.xres < 2)
				g.screen.xres = 2;
			reshapeWindow(g.screen.xres, g.screen.yres);
		}
		return;
	}
	if (key == XK_Right) {
		if (!(shift+ctrl)) {
			//shift and ctrl are not used to navigate lists
			if (g.mode == MODE_FILES) {
				files.currentSceneIncrement(g.filesRowCol[0]);
				return;
			}
		}
		if (shift) {
			//http://stackoverflow.com/questions/3806872/window-position-in-xlib
			XWindowAttributes xwa;
			int x, y;
			Window child;
			Window root = DefaultRootWindow(dpy);
			XTranslateCoordinates(dpy, win, root, 0, 0, &x, &y, &child );
			XGetWindowAttributes(dpy, win, &xwa);
			//printf("window position: %i %i\n", x-xwa.x, y-xwa.y);
			XMoveWindow(dpy, win, x-xwa.x+8, y-xwa.y-28);		
		} else {
			g.screen.xres += (1 + shift*9 + ctrl*19);
			if (g.screen.xres > 2000)
				g.screen.xres = 2000;
			reshapeWindow(g.screen.xres, g.screen.yres);
		}
		return;
	}
	if (key == XK_Return) {
		if (g.mode == MODE_FILES) {
			//If current file is a subdirectory, change dirs.
			if (files.dirWasSelected()) {
				//g.mode = MODE_FILES;
				files.setNSceneFiles(0);
				files.getFiles();
				return;
			}
			setTitle(0);
			glViewport(0, 0, g.screen.xres, g.screen.yres);
			glClear(GL_COLOR_BUFFER_BIT);
			glXSwapBuffers(dpy, win);
			g.mode=0;
			return;
		}
		if (g.mode == MODE_WINDOW_SIZE) {
			int x,y;
			getCurrentWinsize(&x, &y);
			reshapeWindow(x, y);
			glClear(GL_COLOR_BUFFER_BIT);
			//glXSwapBuffers(dpy, win);
			g.mode=0;
			return;
		}
		if (g.mode == MODE_ANTIALIAS) {
			g.mode = g.prevmode;
			g.aaOn = 0;
			glRasterPos2i(0,0);
			glDrawPixels(g.screen.xres, g.screen.yres,
							GL_RGB, GL_FLOAT, (const GLvoid *)g.screendata);
			g.updateScreen=1;
		}
		return;
	}
	if (key == XK_grave) {
		if (g.mode == MODE_INTERVAL) {
			g.renderInterval = 1;
			g.mode=0;
			setTitle(0.0);
			return;
		}
	}
	if (key >= XK_1 && key <= XK_8) {
		if (g.mode == MODE_INTERVAL) {
			g.renderInterval = 1;
			switch(key) {
				case XK_1: g.renderInterval=2; break;
				case XK_2: g.renderInterval=4; break;
				case XK_3: g.renderInterval=8; break;
				case XK_4: g.renderInterval=16; break;
				case XK_5: g.renderInterval=32; break;
				case XK_6: g.renderInterval=64; break;
				case XK_7: g.renderInterval=128; break;
				case XK_8: g.renderInterval=256; break;
			}
			g.mode=0;
			setTitle(0.0);
			return;
		}
		if (g.mode == MODE_ANTIALIAS) {
			g.aaOn = 0;
			g.aaCount = 0;
			g.aaTolerance = 0.01;
			switch(key) {
				case XK_1: g.aaCount =   1; g.aaTolerance = 0.01; break;
				case XK_2: g.aaCount =   5; g.aaTolerance = 0.01; break;
				case XK_3: g.aaCount =  20; g.aaTolerance = 0.01; break;
				case XK_4: g.aaCount =  80; g.aaTolerance = 0.01; break;
				case XK_5: g.aaCount = 200; g.aaTolerance = 0.01; break;
				case XK_6: break;
				case XK_7: break;
			}
			if (g.aaCount > 0) {
				g.aaOn = 1;
			}
			g.mode=0;
			glRasterPos2i(0,0);
			glDrawPixels(g.screen.xres, g.screen.yres,
							GL_RGB, GL_FLOAT, (const GLvoid *)g.screendata);
			g.updateScreen=1;
			return;
		}
		g.traceResolution = pow(2, (key-XK_1));
		setTitle(0);
		return;
	}
	if (key == XK_r || key == XK_c || key == XK_F5) {
		//Log("R/C/F5 pressed\n");
		if (!files.currentIsDir()) {
			g.mode = MODE_NONE;
			castRays(g.traceResolution);
		}
		return;
	}
	if (key == XK_t) {
		int i,j;
		struct timespec timerstart, timerend;
		Vec pix={0,0,0};
		g.mode = MODE_NONE;
		clock_gettime(CLOCK_REALTIME, &timerstart);
		for (i=0; i<g.screen.yres; i++) {
			for (j=0; j<g.screen.xres; j++) {
				pix[0] = 1.0;
				pix[1] += 0.0000006;
				pix[2] += 0.000001;
				setPixel(j,i,pix);
			}
			//render just one raster line
			renderViewport(i,g.screen.xres,1);
			glXSwapBuffers(dpy, win);
		}
		clock_gettime(CLOCK_REALTIME, &timerend);
		setTitle(timediff(&timerstart, &timerend));
		return;
	}
	if (key == XK_z) {
		if (ctrl)
			g.done=1;
		return;
	}
	if (key == XK_Escape) {
		//Log("Esc was pressed.\n");
		if (g.mode != MODE_NONE) {
			glClear(GL_COLOR_BUFFER_BIT);
			g.mode = MODE_NONE;
			g.updateScreen=1;
			return;
		}
		if (g.tracing) {
			g.tracing=0;
			return;
		}
		if (ctrl)
			g.done=1;
		return;
	}
}

#include "vector.h"
//#define CS_COMPUTERS

void checkMouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;
	//
	switch (e->type) {
		case ButtonRelease:
			lbutton=0;
			rbutton=0;
			if (g.rFlag) {
				//a rectangle has just been dragged.
				//Log("\n\n----> dragged <---\n\n");
				//printf("-->dragged<--\n");
				//#ifdef CS_COMPUTERS
				if (g.version==2)
					g.r1[3]--;
				//#endif

				/*------------------------------------------------------------
				Experiment
				g.cFlag=0;
				//if rectangle is a point
				int yd = g.r1[3] - g.r1[1];
				int xd = g.r1[2] - g.r1[0];
				printf("xd: %i  yd: %i\n",xd,yd);
				if (yd == 0 && xd == 0) {
					//rectangle area is zero
					//make a new camera "at" vector.
					int px = g.r1[0];
					int py = g.r1[1];
					printf("px: %i  py: %i\n",px,py);

					Flt ty = 1.0 / (g.screen.fyres - 1.0);
					Flt tx = 1.0 / (g.screen.fxres - 1.0);
					Flt viewAnglex, aspectRatio;
					Flt frustumheight, frustumwidth;
					Vec dir, left, up, out;

					VecNormalize(g.studio.up);
					VecSub(g.studio.at, g.studio.from, out);
					VecNormalize(out);
					aspectRatio = g.screen.fxres / g.screen.fyres;
					viewAnglex = degreesToRadians(g.studio.angle*0.5);
					frustumwidth = tan(viewAnglex);
					frustumheight = frustumwidth / aspectRatio;
					//compute the left and up vectors...
					VecCross(out, g.studio.up, left);
					VecNormalize(left);
					VecCross(out, left, up);
					//
					//Define these temp variables above (outside the loops)
					VecComb(-frustumheight * (2.0 * (Flt)py*ty - 1.0), up,
					         frustumwidth  * (2.0 * (Flt)px*tx - 1.0), left,
					         dir);
					printf("dir: %lf %lf %lf\n",dir[0],dir[1],dir[2]);
					g.cat[0] = (dir[0] + out[0])*10.0;
					g.cat[1] = (dir[1] + out[1])*10.0;
					g.cat[2] = (dir[2] + out[2])*10.0;
					printf("g.cat: %lf %lf %lf\n",g.cat[0],g.cat[1],g.cat[2]);
					g.cFlag=1;
					printf("cFlag is set\n");

					g.r1[0] = 0;
					g.r1[3] = 0;
					g.r1[2] = g.screen.xres-1;
					g.r1[1] = g.screen.yres-1;
				}
				------------------------------------------------------------*/
				
				traceRectangle();
			}
			return;
		case ButtonPress:
			if (e->xbutton.button==3) {
				//right-button pressed.
				g.rspot[0] = e->xbutton.x;
				g.rspot[1] = g.screen.yres - e->xbutton.y;
				g.r1[0] = g.rspot[0];
				g.r1[2] = g.rspot[0] + 1;
				g.r1[3] = g.rspot[1];
				g.r1[1] = g.rspot[1] + 1;
				g.cFlag=1;
				g.rFlag=1;
				traceRectangle();
			}
			if (e->xbutton.button==1) {
				//left-button pressed.
				lbutton = 1;
				g.rspot[0] = e->xbutton.x;
				g.rspot[1] = g.screen.yres - e->xbutton.y;
				g.r1[0] =
				g.r1[2] = g.rspot[0];
				g.r1[1] =
				g.r1[3] = g.rspot[1];
				g.rFlag=1;
			}
	}
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		//mouse moved
		int i,j,tmp;
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		if (lbutton) {
			//Left button is down
			//save previous rectangle.
			//we will overwrite this area when rectangle changes.
			g.r2[0] = g.r1[0];
			g.r2[1] = g.r1[1];
			g.r2[2] = g.r1[2];
			g.r2[3] = g.r1[3];
			//get new
			//this is our rectangle.
			g.r1[0] = g.rspot[0];
			g.r1[1] = g.rspot[1];
			g.r1[2] = savex;
			g.r1[3] = g.screen.yres - savey;
			//but it might be backwards or upside-down.
			if (g.r1[0] > g.r1[2]) {
				tmp = g.r1[0];
				g.r1[0] = g.r1[2];
				g.r1[2] = tmp;
			}
			if (g.r1[3] > g.r1[1]) {
				tmp = g.r1[3];
				g.r1[3] = g.r1[1];
				g.r1[1] = tmp;
			}
			//now check rectangle boundries.
			if (g.r1[0] < 0) g.r1[0] = 0;
			if (g.r1[2] >= g.screen.xres) g.r1[2] = g.screen.xres-1;
			if (g.r1[3] < 0) g.r1[3] = 0;
			if (g.r1[1] >= g.screen.yres) g.r1[1] = g.screen.yres-1;
			//
			//cover up old rectangle lines.
			int r0 = g.r2[0],
				r1 = g.r2[1],
				r2 = g.r2[2],
				r3 = g.r2[3];
			int x3=g.screen.xres*3;
			glBegin(GL_POINTS);
			//top&bot
			//#ifdef CS_COMPUTERS
			//int t;
			//#endif //CS_COMPUTERS
			j = r0;
			while (j<=r2) {
				glColor3fv(g.screendata+(r3*x3+j*3-3));
				glVertex2i(j, r3);
				//#ifdef CS_COMPUTERS
				if (g.version==2) {
					int t = r3-1;
					if (t >= 0 && t < g.screen.yres) {
						glColor3fv(g.screendata+(t*x3+j*3-3));
						glVertex2i(j, t);
					}
				}
				//#endif //CS_COMPUTERS
				glColor3fv(g.screendata+(r1*x3+j*3-3));
				glVertex2i(j, r1);
				//#ifdef CS_COMPUTERS
				if (g.version==2) {
					int t = r1-1;
					if (t >= 0 && t < g.screen.yres) {
						glColor3fv(g.screendata+(t*x3+j*3-3));
						glVertex2i(j, t);
					}
				}
				//#endif //CS_COMPUTERS
				j++;
			}
			//left&right,  r3 < r1
			i = r3;
			while (i<r1) {
				glColor3fv(g.screendata+(i*x3+r0*3-3));
				glVertex2i(r0, i);
				//#ifdef CS_COMPUTERS
				if (g.version==2) {
					int t = r0-1;
					if (t >= 0 && t < g.screen.xres) {
						glColor3fv(g.screendata+(i*x3+t*3-3));
						glVertex2i(t, i);
					}
				}
				//#endif //CS_COMPUTERS
				glColor3fv(g.screendata+(i*x3+r2*3-3));
				glVertex2i(r2, i);
				//#ifdef CS_COMPUTERS
				if (g.version==2) {
					int t = r2-1;
					if (t >= 0 && t < g.screen.xres) {
						glColor3fv(g.screendata+(i*x3+t*3-3));
						glVertex2i(t, i);
					}
				}
				//#endif //CS_COMPUTERS
				i++;
			}
			glEnd();
			//show dragged rectangle...
			glEnable(GL_LINE_STIPPLE);
			glColor3ub(0,0,0);
			glLineStipple(1, 0xAAAA);
			glBegin(GL_LINE_LOOP);
				glVertex2i(g.r1[0],g.r1[1]);
				glVertex2i(g.r1[2],g.r1[1]);
				glVertex2i(g.r1[2],g.r1[3]);
				glVertex2i(g.r1[0],g.r1[3]);
				glVertex2i(g.r1[0],g.r1[1]);
			glEnd();
			glColor3ub(255,255,255);
			glLineStipple(1, 0x5555);
			glBegin(GL_LINE_LOOP);
				glVertex2i(g.r1[0],g.r1[1]);
				glVertex2i(g.r1[2],g.r1[1]);
				glVertex2i(g.r1[2],g.r1[3]);
				glVertex2i(g.r1[0],g.r1[3]);
				glVertex2i(g.r1[0],g.r1[1]);
			glEnd();
			glDisable(GL_LINE_STIPPLE);
			g.updateScreen=1;
			g.rFlag=1;
		}
	}
}

void floatdataToChar(float *cubeBits, unsigned char *uptr, int xres, int yres)
{
	//Convert float values to unsigned char
	int i,j,k;
	unsigned char *dptr = uptr;
	float f, *p = cubeBits;
	for (i=0; i<yres; i++) {
		p = cubeBits + ((yres-1-i) * xres * 3);
		for (j=0; j<xres; j++) {
			for (k=0; k<3; k++) {
				f = *p;
				f = f<0.0f ? 0.0f : f;
				f = f>1.0f ? 1.0f : f;
				*dptr = (char)(int)(f*254.999);
				dptr++;
				p++;
			}
		}
	}
}

void copyR1R2() {
	g.r2[0] = g.r1[0];
	g.r2[1] = g.r1[1];
	g.r2[2] = g.r1[2];
	g.r2[3] = g.r1[3];
}

void saveSnapshot()
{
	printf("saveSnapshot()...\n");
	if (!g.readyToPrint) {
		printf("not ready to print\n");
		return;
	}
	//
	//Use the scene filename to make an image filename.
	char fname[256];
	char xname[256];
	//printf("files.getFilePath(): **%s**\n",files.getFilePath());
	//printf("files.getCurrentSceneName(): **%s**\n",files.getCurrentSceneName());
	sprintf(fname, "%s%s", files.getFilePath(), files.getCurrentSceneName());
	//printf("fname: **%s**\n",fname);
	replaceFileExtension(fname, ".ppm");
	//printf("image name: **%s**\n",fname);
	//Does this file exist already?
	int i=1;
	while ((access(fname, F_OK) != -1) && i++ < 20) {
		//look for new name...
		sprintf(fname, "%s%s", files.getFilePath(),
		                       files.getCurrentSceneName());
		char *ptr = strstr(fname+1, ".");
		if (ptr) *ptr = '\0';
		sprintf(xname, "%s%i.ppm", fname, i);
		strcpy(fname, xname);
		printf("try this name **%s**\n", fname);
	}
	printf("file being created: **%s**\n", fname);
	Ppmimage *image = ppm6CreateImage(g.screen.xres, g.screen.yres);
	floatdataToChar(g.screendata, (unsigned char *)image->data,
	                g.screen.xres, g.screen.yres);
	ppm6SaveImage(image, fname);
	ppm6CleanupImage(image);
	//
	//save a low-quality jpg also.
	strcpy(xname, fname);
	replaceFileExtension(xname, ".jpg");
	int ret;
	char ts[128];
	sprintf(ts,"convert %s -quality 50%% %s",fname,xname);
	ret = system(ts);
	printf("%s also created.\n", xname);
	if (ret){}
}

void saveTempFile(Ray *ray, Flt angle)
{
	Vec at;
	at[0] = ray->o[0] + ray->d[0] * 100.0;
	at[1] = ray->o[1] + ray->d[1] * 100.0;
	at[2] = ray->o[2] + ray->d[2] * 100.0;
	//
	char ts[256];
	char name[40]="zztmp.b";
	int n=1;
	if (strcmp(files.getCurrentSceneName(), name) == 0) {
		strcpy(name, "zz2tmp.b");
		n++;
	}
	sprintf(ts,"%s%s", files.getFilePath(),
	                   files.getCurrentSceneName());
	FILE *fpi = fopen(ts, "r");
	if (!fpi) exit(1);
	sprintf(ts,"%s%s", files.getFilePath(), name);
	FILE *fpo = fopen(ts,"w");
	if (!fpo) exit(1);
	if (fgets(ts, 256, fpi)) {}
	while (!feof(fpi)) {
		fprintf(fpo, "%s", ts);
		if (fgets(ts, 256, fpi)) {}
	}
	fprintf(fpo,"from: %lf %lf %lf\n",ray->o[0],ray->o[1],ray->o[2]);
	fprintf(fpo,"  at: %lf %lf %lf\n",at[0],at[1],at[2]);
	fprintf(fpo,"angle: %lf\n", angle * 0.5);
	fclose(fpo);
	fclose(fpi);
	if (n > 1) {
		sprintf(ts, "%szztmp.b", files.getFilePath());
		remove(ts);
		char ts2[256];
		sprintf(ts2, "%szz2tmp.b", files.getFilePath());
		rename(ts2, ts);
	}
}

void traceRectangle()
{
	if (g.cFlag) {
		//right button pressed, zooming in.
		//How zoom-in works...
		//1. Create a tracing rectangle at the pixel where clicked.
		//2. When pixel is traced, save the ray shot through the pixel.
		//   Use the ray to create a new camera position and direction.
		//   This is from: and at:
		//3. Reduce the viewing angle. This is what creates the zoom effect.
		//4. Copy the current scene file to a tmp file, and insert the
		//   components from:, to:, and angle:.
		//5. Send the new file to castRays() and see the zoomed view.
		//
		castRays(g.traceResolution);
		char ts[256];
		strcpy(ts, "zztmp.b");
		files.setCurrentSceneName(ts);
		//printf("set. tracing **%s**\n",files.getCurrentSceneName());
	}
	castRays(g.traceResolution);
	g.updateScreen=1;
}

char *hms(char *str, double dsec)
{
	int sec = (int)dsec;
	int hours = sec / 3600;
	sec = sec % 3600;
	int min = sec / 60;
	sec = sec % 60;
	sprintf(str,"%02i:%02i:%02i",hours, min, sec);
	return str;
}
























